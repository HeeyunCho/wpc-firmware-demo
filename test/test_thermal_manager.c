#include "th.h"
#include "thermal_manager.h"

static void test_no_derating_below_start(void)
{
    TH_ASSERT_EQ_INT(100u, thermal_derating_percent(25));
    TH_ASSERT_EQ_INT(100u, thermal_derating_percent(44));
    TH_ASSERT_EQ_INT(100u, thermal_derating_percent(45));
}

static void test_derating_linear_midpoint(void)
{
    TH_ASSERT_EQ_INT(96u, thermal_derating_percent(46));
    TH_ASSERT_EQ_INT(72u, thermal_derating_percent(52));
    TH_ASSERT_EQ_INT(44u, thermal_derating_percent(59));
}

static void test_derating_floor(void)
{
    TH_ASSERT_EQ_INT(40u, thermal_derating_percent(60));
    TH_ASSERT_EQ_INT(40u, thermal_derating_percent(69));
}

static void test_shutdown_at_threshold(void)
{
    thermal_ctx_t ctx;
    thermal_init(&ctx);
    /* Exactly 70 C must shut power transfer down (SYS safety requirement). */
    TH_ASSERT_EQ_INT(0u, thermal_derating_percent(WPC_THERMAL_SHUTDOWN_C));
    TH_ASSERT_EQ_INT(THERMAL_SHUTDOWN, thermal_update(&ctx, WPC_THERMAL_SHUTDOWN_C));
    TH_ASSERT_EQ_INT(0u, thermal_power_limit_mw(&ctx));
}

static void test_shutdown_hysteresis_resume(void)
{
    thermal_ctx_t ctx;
    thermal_init(&ctx);
    TH_ASSERT_EQ_INT(THERMAL_SHUTDOWN, thermal_update(&ctx, 75));
    TH_ASSERT_EQ_INT(THERMAL_SHUTDOWN, thermal_update(&ctx, 65));
    TH_ASSERT_EQ_INT(THERMAL_SHUTDOWN, thermal_update(&ctx, 56));
    TH_ASSERT_EQ_INT(THERMAL_DERATING, thermal_update(&ctx, 55));
    TH_ASSERT_EQ_INT(60u, ctx.power_percent);
    TH_ASSERT_EQ_INT(1u, ctx.shutdown_count);
}

static void test_sensor_fault_shutdown(void)
{
    thermal_ctx_t ctx;
    thermal_init(&ctx);
    TH_ASSERT_EQ_INT(THERMAL_SENSOR_FAULT, thermal_update(&ctx, 200));
    TH_ASSERT_EQ_INT(0u, thermal_power_limit_mw(&ctx));
    TH_ASSERT_EQ_INT(THERMAL_SENSOR_FAULT, thermal_update(&ctx, -60));
    TH_ASSERT_EQ_INT(THERMAL_NORMAL, thermal_update(&ctx, 30));
}

static void test_power_limit_mw(void)
{
    thermal_ctx_t ctx;
    thermal_init(&ctx);
    TH_ASSERT_EQ_INT(15000u, thermal_power_limit_mw(&ctx));
    (void)thermal_update(&ctx, 60);
    TH_ASSERT_EQ_INT(6000u, thermal_power_limit_mw(&ctx));
}

void run_thermal_manager_tests(void)
{
    th_begin_suite("thermal_manager");
    TH_RUN(test_no_derating_below_start);
    TH_RUN(test_derating_linear_midpoint);
    TH_RUN(test_derating_floor);
    TH_RUN(test_shutdown_at_threshold);
    TH_RUN(test_shutdown_hysteresis_resume);
    TH_RUN(test_sensor_fault_shutdown);
    TH_RUN(test_power_limit_mw);
}
