#include "th.h"
#include "fod_detector.h"

static void test_q_factor_threshold(void)
{
    /* 20 % drop allowed: Q_ref 100 -> limit 80. */
    TH_ASSERT_EQ_INT(FOD_OK, fod_check_q_factor(&FOD_DEFAULT_CONFIG, 100u, 80u));
    TH_ASSERT_EQ_INT(FOD_DETECTED, fod_check_q_factor(&FOD_DEFAULT_CONFIG, 100u, 79u));
}

static void test_q_factor_nominal_no_fo(void)
{
    TH_ASSERT_EQ_INT(FOD_OK, fod_check_q_factor(&FOD_DEFAULT_CONFIG, 60u, 58u));
    TH_ASSERT_EQ_INT(FOD_OK, fod_check_q_factor(&FOD_DEFAULT_CONFIG, 60u, 65u));
}

static void test_q_factor_invalid_reference(void)
{
    TH_ASSERT_EQ_INT(FOD_INVALID_INPUT, fod_check_q_factor(&FOD_DEFAULT_CONFIG, 0u, 50u));
    TH_ASSERT_EQ_INT(FOD_INVALID_INPUT, fod_check_q_factor(0, 100u, 50u));
}

static void test_power_loss_dynamic_threshold(void)
{
    /* 250 mW + 3 % of P_rx */
    TH_ASSERT_EQ_INT(250u, fod_power_loss_threshold_mw(&FOD_DEFAULT_CONFIG, 0u));
    TH_ASSERT_EQ_INT(700u, fod_power_loss_threshold_mw(&FOD_DEFAULT_CONFIG, 15000u));
}

static void test_power_loss_below_threshold(void)
{
    fod_state_t s;
    fod_init(&s, &FOD_DEFAULT_CONFIG);
    /* 15 W received, 700 mW loss == threshold -> OK */
    TH_ASSERT_EQ_INT(FOD_OK, fod_update_power_loss(&s, 15700u, 15000u));
}

static void test_power_loss_debounce(void)
{
    fod_state_t s;
    fod_init(&s, &FOD_DEFAULT_CONFIG);
    TH_ASSERT_EQ_INT(FOD_SUSPECTED, fod_update_power_loss(&s, 16000u, 15000u));
    TH_ASSERT_EQ_INT(FOD_SUSPECTED, fod_update_power_loss(&s, 16000u, 15000u));
    TH_ASSERT_EQ_INT(FOD_DETECTED, fod_update_power_loss(&s, 16000u, 15000u));
    /* latched until reset, even if loss disappears */
    TH_ASSERT_EQ_INT(FOD_DETECTED, fod_update_power_loss(&s, 15000u, 15000u));
    fod_reset(&s);
    TH_ASSERT_EQ_INT(FOD_OK, fod_update_power_loss(&s, 15000u, 15000u));
}

static void test_power_loss_counter_reset(void)
{
    fod_state_t s;
    fod_init(&s, &FOD_DEFAULT_CONFIG);
    TH_ASSERT_EQ_INT(FOD_SUSPECTED, fod_update_power_loss(&s, 16000u, 15000u));
    TH_ASSERT_EQ_INT(FOD_SUSPECTED, fod_update_power_loss(&s, 16000u, 15000u));
    TH_ASSERT_EQ_INT(FOD_OK, fod_update_power_loss(&s, 15200u, 15000u));
    TH_ASSERT_EQ_INT(FOD_SUSPECTED, fod_update_power_loss(&s, 16000u, 15000u));
}

static void test_power_loss_implausible_rx(void)
{
    fod_state_t s;
    fod_init(&s, &FOD_DEFAULT_CONFIG);
    TH_ASSERT_EQ_INT(FOD_INVALID_INPUT, fod_update_power_loss(&s, 10000u, 12000u));
}

void run_fod_detector_tests(void)
{
    th_begin_suite("fod_detector");
    TH_RUN(test_q_factor_threshold);
    TH_RUN(test_q_factor_nominal_no_fo);
    TH_RUN(test_q_factor_invalid_reference);
    TH_RUN(test_power_loss_dynamic_threshold);
    TH_RUN(test_power_loss_below_threshold);
    TH_RUN(test_power_loss_debounce);
    TH_RUN(test_power_loss_counter_reset);
    TH_RUN(test_power_loss_implausible_rx);
}
