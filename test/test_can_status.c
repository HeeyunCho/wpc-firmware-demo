#include "th.h"
#include "can_status.h"

#include <string.h>

static can_status_signals_t sample(void)
{
    can_status_signals_t s;
    memset(&s, 0, sizeof s);
    s.charge_state = CAN_CHARGE_CHARGING;
    s.fod_active = false;
    s.nfc_pause = true;
    s.thermal_state = 1u;
    s.power_mw = 12300u;
    s.coil_temp_c = 48;
    s.fault_bits = CAN_FAULT_COMM;
    s.phone_left_warning = true;
    return s;
}

static void test_pack_charging_state(void)
{
    can_status_tx_t tx;
    can_status_signals_t s = sample();
    uint8_t f[CAN_STATUS_DLC];
    can_status_init(&tx);
    can_status_pack(&tx, &s, f);
    /* state 2 | nfc 0x20 | thermal 1<<6 */
    TH_ASSERT_EQ_INT(0x62u, f[0]);
    TH_ASSERT_EQ_INT(0x01u, f[4]);
}

static void test_pack_power_scaling(void)
{
    can_status_tx_t tx;
    can_status_signals_t s = sample();
    uint8_t f[CAN_STATUS_DLC];
    can_status_init(&tx);
    can_status_pack(&tx, &s, f);
    TH_ASSERT_EQ_INT(123u, f[1]);
}

static void test_power_saturation(void)
{
    can_status_tx_t tx;
    can_status_signals_t s = sample();
    uint8_t f[CAN_STATUS_DLC];
    can_status_init(&tx);
    s.power_mw = 40000u;
    can_status_pack(&tx, &s, f);
    TH_ASSERT_EQ_INT(255u, f[1]);
}

static void test_pack_temperature_offset(void)
{
    can_status_tx_t tx;
    can_status_signals_t s = sample();
    uint8_t f[CAN_STATUS_DLC];
    can_status_init(&tx);
    can_status_pack(&tx, &s, f);
    TH_ASSERT_EQ_INT(88u, f[2]);
    s.coil_temp_c = -50;
    can_status_pack(&tx, &s, f);
    TH_ASSERT_EQ_INT(0u, f[2]);
}

static void test_rolling_counter_wraps(void)
{
    can_status_tx_t tx;
    can_status_signals_t s = sample();
    uint8_t f[CAN_STATUS_DLC];
    int i;
    can_status_init(&tx);
    for (i = 0; i < 16; i++) {
        can_status_pack(&tx, &s, f);
        TH_ASSERT_EQ_INT(i, f[6]);
    }
    can_status_pack(&tx, &s, f);
    TH_ASSERT_EQ_INT(0, f[6]);
}

static void test_checksum(void)
{
    can_status_tx_t tx;
    can_status_signals_t s = sample();
    can_status_signals_t out;
    uint8_t f[CAN_STATUS_DLC];
    can_status_init(&tx);
    can_status_pack(&tx, &s, f);
    TH_ASSERT_EQ_INT(can_status_checksum(f), f[7]);
    f[1] ^= 0x01u; /* corrupt one bit */
    TH_ASSERT(!can_status_unpack(f, &out, 0));
}

static void test_pack_unpack_roundtrip(void)
{
    can_status_tx_t tx;
    can_status_signals_t s = sample();
    can_status_signals_t out;
    uint8_t f[CAN_STATUS_DLC];
    uint8_t alive = 0xFFu;
    can_status_init(&tx);
    can_status_pack(&tx, &s, f);
    TH_ASSERT(can_status_unpack(f, &out, &alive));
    TH_ASSERT_EQ_INT(s.charge_state, out.charge_state);
    TH_ASSERT_EQ_INT(s.nfc_pause, out.nfc_pause);
    TH_ASSERT_EQ_INT(s.fod_active, out.fod_active);
    TH_ASSERT_EQ_INT(s.thermal_state, out.thermal_state);
    TH_ASSERT_EQ_INT(s.power_mw, out.power_mw);
    TH_ASSERT_EQ_INT(s.coil_temp_c, out.coil_temp_c);
    TH_ASSERT_EQ_INT(s.fault_bits, out.fault_bits);
    TH_ASSERT_EQ_INT(s.phone_left_warning, out.phone_left_warning);
    TH_ASSERT_EQ_INT(0u, alive);
}

void run_can_status_tests(void)
{
    th_begin_suite("can_status");
    TH_RUN(test_pack_charging_state);
    TH_RUN(test_pack_power_scaling);
    TH_RUN(test_power_saturation);
    TH_RUN(test_pack_temperature_offset);
    TH_RUN(test_rolling_counter_wraps);
    TH_RUN(test_checksum);
    TH_RUN(test_pack_unpack_roundtrip);
}
