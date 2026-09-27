/**
 * @file thermal_manager.h
 * @brief Coil/charger thermal protection: derating curve and shutdown.
 *
 * Derating curve (coil temperature T in deg C):
 *   T <  45            : 100 % of rated power
 *   45 <= T < 60       : linear, -4 %/K  (100 % at 45 C -> 40 % at 60 C)
 *   60 <= T < 70       : 40 % floor
 *   T >= 70            : SHUTDOWN (0 %), latched until T <= 55 C
 *   T outside [-40,150]: sensor fault -> SHUTDOWN
 */
#ifndef THERMAL_MANAGER_H
#define THERMAL_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

#define WPC_THERMAL_DERATE_START_C   45
#define WPC_THERMAL_DERATE_END_C     60
#define WPC_THERMAL_FLOOR_PERCENT    40u
#define WPC_THERMAL_SHUTDOWN_C       70
#define WPC_THERMAL_RESUME_C         55
#define WPC_THERMAL_SENSOR_MIN_C    (-40)
#define WPC_THERMAL_SENSOR_MAX_C     150
#define WPC_RATED_POWER_MW           15000u

typedef enum {
    THERMAL_NORMAL = 0,
    THERMAL_DERATING,
    THERMAL_SHUTDOWN,
    THERMAL_SENSOR_FAULT
} thermal_state_t;

typedef struct {
    thermal_state_t state;
    uint8_t         power_percent;
    uint16_t        shutdown_count;   /**< diagnostic counter (DTC support) */
} thermal_ctx_t;

void            thermal_init(thermal_ctx_t *ctx);
uint8_t         thermal_derating_percent(int16_t temp_c);
thermal_state_t thermal_update(thermal_ctx_t *ctx, int16_t temp_c);
uint32_t        thermal_power_limit_mw(const thermal_ctx_t *ctx);

#endif /* THERMAL_MANAGER_H */
