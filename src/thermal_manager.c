#include "thermal_manager.h"
#include "wpc_config.h"

#include <stddef.h>

static bool temp_is_plausible(int16_t temp_c)
{
    return temp_c >= WPC_THERMAL_SENSOR_MIN_C && temp_c <= WPC_THERMAL_SENSOR_MAX_C;
}

static bool temp_is_shutdown(int16_t temp_c)
{
#if WPC_DEMO_THERMAL_BUG
    /* DEMO BUG: off-by-one - exactly 70 C does not trigger shutdown. */
    return temp_c > WPC_THERMAL_SHUTDOWN_C;
#else
    return temp_c >= WPC_THERMAL_SHUTDOWN_C;
#endif
}

void thermal_init(thermal_ctx_t *ctx)
{
    if (ctx == NULL) {
        return;
    }
    ctx->state = THERMAL_NORMAL;
    ctx->power_percent = 100u;
    ctx->shutdown_count = 0u;
}

uint8_t thermal_derating_percent(int16_t temp_c)
{
    if (!temp_is_plausible(temp_c) || temp_is_shutdown(temp_c)) {
        return 0u;
    }
    if (temp_c < WPC_THERMAL_DERATE_START_C) {
        return 100u;
    }
    if (temp_c < WPC_THERMAL_DERATE_END_C) {
        int pct = 100 - 4 * (temp_c - WPC_THERMAL_DERATE_START_C);
        return (uint8_t)pct;
    }
    return WPC_THERMAL_FLOOR_PERCENT;
}

thermal_state_t thermal_update(thermal_ctx_t *ctx, int16_t temp_c)
{
    uint8_t pct;

    if (ctx == NULL) {
        return THERMAL_SENSOR_FAULT;
    }

    if (!temp_is_plausible(temp_c)) {
        if (ctx->state != THERMAL_SENSOR_FAULT) {
            ctx->shutdown_count++;
        }
        ctx->state = THERMAL_SENSOR_FAULT;
        ctx->power_percent = 0u;
        return ctx->state;
    }

    /* Latched shutdown (or recovered sensor): resume only after cooling down. */
    if (ctx->state == THERMAL_SHUTDOWN || ctx->state == THERMAL_SENSOR_FAULT) {
        if (temp_c > WPC_THERMAL_RESUME_C) {
            ctx->state = THERMAL_SHUTDOWN;
            ctx->power_percent = 0u;
            return ctx->state;
        }
    }

    pct = thermal_derating_percent(temp_c);
    if (pct == 0u) {
        if (ctx->state != THERMAL_SHUTDOWN) {
            ctx->shutdown_count++;
        }
        ctx->state = THERMAL_SHUTDOWN;
    } else if (pct < 100u) {
        ctx->state = THERMAL_DERATING;
    } else {
        ctx->state = THERMAL_NORMAL;
    }
    ctx->power_percent = pct;
    return ctx->state;
}

uint32_t thermal_power_limit_mw(const thermal_ctx_t *ctx)
{
    if (ctx == NULL) {
        return 0u;
    }
    return (WPC_RATED_POWER_MW * ctx->power_percent) / 100u;
}
