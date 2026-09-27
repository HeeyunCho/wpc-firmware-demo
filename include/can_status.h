/**
 * @file can_status.h
 * @brief Packing of the WCM_Status CAN frame (ID 0x3A5, DLC 8, cycle 100 ms).
 *
 * Byte | Bits | Signal
 * -----+------+-------------------------------------------------------------
 *  0   | 0..3 | ChargeState (can_charge_state_t)
 *  0   | 4    | FodActive
 *  0   | 5    | NfcPause
 *  0   | 6..7 | ThermalState (0 normal, 1 derating, 2 shutdown, 3 fault)
 *  1   | 0..7 | PowerLevel, 100 mW/bit, 0..25.5 W (saturates)
 *  2   | 0..7 | CoilTemp, 1 C/bit, offset -40 C (saturates at -40 / 215)
 *  3   | 0..7 | FaultBits (CAN_FAULT_*)
 *  4   | 0    | PhoneLeftWarning
 *  5   | -    | reserved (0)
 *  6   | 0..3 | AliveCounter (rolling 0..15)
 *  7   | 0..7 | Checksum = XOR of bytes 0..6 XOR 0x5A
 */
#ifndef CAN_STATUS_H
#define CAN_STATUS_H

#include <stdint.h>
#include <stdbool.h>

#define CAN_STATUS_ID      0x3A5u
#define CAN_STATUS_DLC     8u
#define CAN_CHECKSUM_SEED  0x5Au

#define CAN_FAULT_OVERTEMP     0x01u
#define CAN_FAULT_FOD          0x02u
#define CAN_FAULT_SENSOR       0x04u
#define CAN_FAULT_COMM         0x08u

typedef enum {
    CAN_CHARGE_OFF = 0,
    CAN_CHARGE_IDLE = 1,
    CAN_CHARGE_CHARGING = 2,
    CAN_CHARGE_COMPLETE = 3,
    CAN_CHARGE_PAUSED = 4,
    CAN_CHARGE_FAULT = 5
} can_charge_state_t;

typedef struct {
    can_charge_state_t charge_state;
    bool               fod_active;
    bool               nfc_pause;
    uint8_t            thermal_state;  /**< 0..3 */
    uint32_t           power_mw;
    int16_t            coil_temp_c;
    uint8_t            fault_bits;
    bool               phone_left_warning;
} can_status_signals_t;

typedef struct {
    uint8_t alive_counter;
} can_status_tx_t;

void    can_status_init(can_status_tx_t *tx);
/** Pack signals into out[8], advancing the alive counter. */
void    can_status_pack(can_status_tx_t *tx, const can_status_signals_t *sig, uint8_t out[CAN_STATUS_DLC]);
uint8_t can_status_checksum(const uint8_t data[CAN_STATUS_DLC]);
/** Unpack and verify checksum. Returns false on checksum mismatch. */
bool    can_status_unpack(const uint8_t data[CAN_STATUS_DLC], can_status_signals_t *sig, uint8_t *alive);

#endif /* CAN_STATUS_H */
