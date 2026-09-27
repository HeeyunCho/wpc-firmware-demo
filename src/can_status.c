#include "can_status.h"

#include <stddef.h>
#include <string.h>

void can_status_init(can_status_tx_t *tx)
{
    if (tx != NULL) {
        tx->alive_counter = 0u;
    }
}

uint8_t can_status_checksum(const uint8_t data[CAN_STATUS_DLC])
{
    uint8_t cs = CAN_CHECKSUM_SEED;
    unsigned i;
    for (i = 0u; i < CAN_STATUS_DLC - 1u; i++) {
        cs ^= data[i];
    }
    return cs;
}

static uint8_t encode_power(uint32_t power_mw)
{
    uint32_t raw = power_mw / 100u;
    return (uint8_t)(raw > 255u ? 255u : raw);
}

static uint8_t encode_temp(int16_t temp_c)
{
    int32_t raw = (int32_t)temp_c + 40;
    if (raw < 0) {
        raw = 0;
    } else if (raw > 255) {
        raw = 255;
    }
    return (uint8_t)raw;
}

void can_status_pack(can_status_tx_t *tx, const can_status_signals_t *sig, uint8_t out[CAN_STATUS_DLC])
{
    if (tx == NULL || sig == NULL || out == NULL) {
        return;
    }
    memset(out, 0, CAN_STATUS_DLC);

    out[0] = (uint8_t)(((unsigned)sig->charge_state & 0x0Fu)
                       | (sig->fod_active ? 0x10u : 0u)
                       | (sig->nfc_pause ? 0x20u : 0u)
                       | (((unsigned)sig->thermal_state & 0x03u) << 6));
    out[1] = encode_power(sig->power_mw);
    out[2] = encode_temp(sig->coil_temp_c);
    out[3] = sig->fault_bits;
    out[4] = sig->phone_left_warning ? 0x01u : 0x00u;
    out[5] = 0u;
    out[6] = (uint8_t)(tx->alive_counter & 0x0Fu);
    out[7] = can_status_checksum(out);

    tx->alive_counter = (uint8_t)((tx->alive_counter + 1u) & 0x0Fu);
}

bool can_status_unpack(const uint8_t data[CAN_STATUS_DLC], can_status_signals_t *sig, uint8_t *alive)
{
    if (data == NULL || sig == NULL) {
        return false;
    }
    if (can_status_checksum(data) != data[7]) {
        return false;
    }
    sig->charge_state = (can_charge_state_t)(data[0] & 0x0Fu);
    sig->fod_active = (data[0] & 0x10u) != 0u;
    sig->nfc_pause = (data[0] & 0x20u) != 0u;
    sig->thermal_state = (uint8_t)((data[0] >> 6) & 0x03u);
    sig->power_mw = (uint32_t)data[1] * 100u;
    sig->coil_temp_c = (int16_t)((int16_t)data[2] - 40);
    sig->fault_bits = data[3];
    sig->phone_left_warning = (data[4] & 0x01u) != 0u;
    if (alive != NULL) {
        *alive = (uint8_t)(data[6] & 0x0Fu);
    }
    return true;
}
