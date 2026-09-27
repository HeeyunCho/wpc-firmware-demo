/**
 * @file fod_detector.h
 * @brief Foreign Object Detection (FOD) for the Qi2 transmitter coil.
 *
 * Two complementary methods, as required by the Qi v2 specification:
 *  - Q-factor method (pre-power-transfer): a metal object near the coil
 *    lowers the measured quality factor relative to the receiver's reported
 *    reference Q.
 *  - Power-loss method (during power transfer): transmitted power minus
 *    power reported by the receiver must stay below a (load-dependent)
 *    threshold; a debounced violation stops power transfer.
 */
#ifndef FOD_DETECTOR_H
#define FOD_DETECTOR_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    FOD_OK = 0,          /**< no foreign object */
    FOD_SUSPECTED,       /**< single violation, not yet debounced */
    FOD_DETECTED,        /**< foreign object confirmed: stop power transfer */
    FOD_INVALID_INPUT    /**< implausible measurement */
} fod_result_t;

typedef struct {
    uint8_t  q_drop_percent;      /**< FO if Q_meas < Q_ref * (100 - x) / 100 */
    uint16_t loss_base_mw;        /**< fixed part of power-loss threshold */
    uint8_t  loss_prop_permille;  /**< load-proportional part (per mille of P_rx) */
    uint8_t  debounce_count;      /**< consecutive violations to confirm FO */
} fod_config_t;

typedef struct {
    fod_config_t cfg;
    uint8_t      violations;
    bool         latched;         /**< FO confirmed; cleared only by fod_reset */
} fod_state_t;

/** Default calibration for the 15 W Qi2 MPP coil. */
extern const fod_config_t FOD_DEFAULT_CONFIG;

void         fod_init(fod_state_t *s, const fod_config_t *cfg);
void         fod_reset(fod_state_t *s);
fod_result_t fod_check_q_factor(const fod_config_t *cfg, uint16_t q_ref, uint16_t q_meas);
uint32_t     fod_power_loss_threshold_mw(const fod_config_t *cfg, uint32_t p_rx_mw);
fod_result_t fod_update_power_loss(fod_state_t *s, uint32_t p_tx_mw, uint32_t p_rx_mw);

#endif /* FOD_DETECTOR_H */
