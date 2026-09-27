#include "fod_detector.h"

#include <stddef.h>

const fod_config_t FOD_DEFAULT_CONFIG = {
    /* q_drop_percent     */ 20u,
    /* loss_base_mw       */ 250u,
    /* loss_prop_permille */ 30u,
    /* debounce_count     */ 3u,
};

void fod_init(fod_state_t *s, const fod_config_t *cfg)
{
    if (s == NULL) {
        return;
    }
    s->cfg = (cfg != NULL) ? *cfg : FOD_DEFAULT_CONFIG;
    fod_reset(s);
}

void fod_reset(fod_state_t *s)
{
    if (s == NULL) {
        return;
    }
    s->violations = 0u;
    s->latched = false;
}

fod_result_t fod_check_q_factor(const fod_config_t *cfg, uint16_t q_ref, uint16_t q_meas)
{
    uint32_t limit;

    if (cfg == NULL || q_ref == 0u || cfg->q_drop_percent >= 100u) {
        return FOD_INVALID_INPUT;
    }
    /* Q_meas < Q_ref * (100 - drop) / 100, evaluated without division. */
    limit = (uint32_t)q_ref * (uint32_t)(100u - cfg->q_drop_percent);
    if ((uint32_t)q_meas * 100u < limit) {
        return FOD_DETECTED;
    }
    return FOD_OK;
}

uint32_t fod_power_loss_threshold_mw(const fod_config_t *cfg, uint32_t p_rx_mw)
{
    if (cfg == NULL) {
        return 0u;
    }
    return (uint32_t)cfg->loss_base_mw + (p_rx_mw * cfg->loss_prop_permille) / 1000u;
}

fod_result_t fod_update_power_loss(fod_state_t *s, uint32_t p_tx_mw, uint32_t p_rx_mw)
{
    uint32_t loss;

    if (s == NULL) {
        return FOD_INVALID_INPUT;
    }
    if (s->latched) {
        return FOD_DETECTED;
    }
    /* Receiver cannot receive more than we transmit (plus 10 % calibration margin). */
    if (p_rx_mw > p_tx_mw + p_tx_mw / 10u) {
        return FOD_INVALID_INPUT;
    }

    loss = (p_tx_mw > p_rx_mw) ? (p_tx_mw - p_rx_mw) : 0u;
    if (loss > fod_power_loss_threshold_mw(&s->cfg, p_rx_mw)) {
        if (s->violations < 0xFFu) {
            s->violations++;
        }
        if (s->violations >= s->cfg.debounce_count) {
            s->latched = true;
            return FOD_DETECTED;
        }
        return FOD_SUSPECTED;
    }

    s->violations = 0u;
    return FOD_OK;
}
