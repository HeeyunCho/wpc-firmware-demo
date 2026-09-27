#include "th.h"
#include "nfc_guard.h"

static void test_no_card_power_allowed(void)
{
    nfc_guard_t g;
    nfc_guard_init(&g);
    TH_ASSERT(nfc_guard_update(&g, false));
    TH_ASSERT(nfc_guard_power_allowed(&g));
}

static void test_card_detected_pauses_power(void)
{
    nfc_guard_t g;
    nfc_guard_init(&g);
    TH_ASSERT(!nfc_guard_update(&g, true));
    TH_ASSERT_EQ_INT(NFC_GUARD_PAUSED, g.state);
}

static void test_resume_after_absent_debounce(void)
{
    nfc_guard_t g;
    nfc_guard_init(&g);
    (void)nfc_guard_update(&g, true);
    TH_ASSERT(!nfc_guard_update(&g, false));
    TH_ASSERT(!nfc_guard_update(&g, false));
    TH_ASSERT(nfc_guard_update(&g, false));
}

static void test_flicker_does_not_resume_early(void)
{
    nfc_guard_t g;
    nfc_guard_init(&g);
    (void)nfc_guard_update(&g, true);
    TH_ASSERT(!nfc_guard_update(&g, false));
    TH_ASSERT(!nfc_guard_update(&g, false));
    TH_ASSERT(!nfc_guard_update(&g, true));   /* card re-seen: counter restarts */
    TH_ASSERT(!nfc_guard_update(&g, false));
    TH_ASSERT(!nfc_guard_update(&g, false));
    TH_ASSERT(nfc_guard_update(&g, false));
}

static void test_pause_count_diagnostic(void)
{
    nfc_guard_t g;
    nfc_guard_init(&g);
    (void)nfc_guard_update(&g, true);
    (void)nfc_guard_update(&g, true);   /* same event */
    (void)nfc_guard_update(&g, false);
    (void)nfc_guard_update(&g, false);
    (void)nfc_guard_update(&g, false);
    (void)nfc_guard_update(&g, true);   /* second event */
    TH_ASSERT_EQ_INT(2u, g.pause_events);
}

static void test_null_guard_fails_safe(void)
{
    TH_ASSERT(!nfc_guard_update(0, false));
    TH_ASSERT(!nfc_guard_power_allowed(0));
}

void run_nfc_guard_tests(void)
{
    th_begin_suite("nfc_guard");
    TH_RUN(test_no_card_power_allowed);
    TH_RUN(test_card_detected_pauses_power);
    TH_RUN(test_resume_after_absent_debounce);
    TH_RUN(test_flicker_does_not_resume_early);
    TH_RUN(test_pause_count_diagnostic);
    TH_RUN(test_null_guard_fails_safe);
}
