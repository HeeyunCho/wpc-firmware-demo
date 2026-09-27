#include "th.h"
#include "phone_reminder.h"

static bool step(phone_reminder_t *r, bool ign, bool phone, bool door, uint32_t t)
{
    phone_reminder_inputs_t in;
    in.ignition_on = ign;
    in.phone_present = phone;
    in.driver_door_open = door;
    in.now_ms = t;
    return phone_reminder_update(r, &in);
}

static void test_reminder_on_ignition_off_with_phone(void)
{
    phone_reminder_t r;
    phone_reminder_init(&r);
    TH_ASSERT(!step(&r, true, true, false, 0u));
    TH_ASSERT(!step(&r, false, true, false, 1000u));   /* armed */
    TH_ASSERT(step(&r, false, true, true, 5000u));     /* door opens -> warn */
}

static void test_no_reminder_without_phone(void)
{
    phone_reminder_t r;
    phone_reminder_init(&r);
    (void)step(&r, true, false, false, 0u);
    TH_ASSERT(!step(&r, false, false, false, 1000u));
    TH_ASSERT(!step(&r, false, false, true, 2000u));
}

static void test_reminder_cancelled_on_phone_removal(void)
{
    phone_reminder_t r;
    phone_reminder_init(&r);
    (void)step(&r, true, true, false, 0u);
    (void)step(&r, false, true, false, 1000u);
    TH_ASSERT(!step(&r, false, false, false, 2000u));  /* phone taken */
    TH_ASSERT(!step(&r, false, true, true, 3000u));
}

static void test_reminder_timeout(void)
{
    phone_reminder_t r;
    phone_reminder_init(&r);
    (void)step(&r, true, true, false, 0u);
    (void)step(&r, false, true, false, 1000u);
    TH_ASSERT(!step(&r, false, true, true, 1000u + PHONE_REMINDER_TIMEOUT_MS + 1u));
}

static void test_reminder_only_once_per_cycle(void)
{
    phone_reminder_t r;
    phone_reminder_init(&r);
    (void)step(&r, true, true, false, 0u);
    (void)step(&r, false, true, false, 1000u);
    TH_ASSERT(step(&r, false, true, true, 2000u));
    TH_ASSERT(!step(&r, false, true, false, 3000u));   /* door closed */
    TH_ASSERT(!step(&r, false, true, true, 4000u));    /* reopened: no repeat */
    /* new ignition cycle re-enables */
    (void)step(&r, true, true, false, 5000u);
    (void)step(&r, false, true, false, 6000u);
    TH_ASSERT(step(&r, false, true, true, 7000u));
}

static void test_no_reminder_phone_placed_after_ignition_off(void)
{
    phone_reminder_t r;
    phone_reminder_init(&r);
    (void)step(&r, true, false, false, 0u);
    (void)step(&r, false, false, false, 1000u);
    TH_ASSERT(!step(&r, false, true, false, 2000u));
    TH_ASSERT(!step(&r, false, true, true, 3000u));
}

void run_phone_reminder_tests(void)
{
    th_begin_suite("phone_reminder");
    TH_RUN(test_reminder_on_ignition_off_with_phone);
    TH_RUN(test_no_reminder_without_phone);
    TH_RUN(test_reminder_cancelled_on_phone_removal);
    TH_RUN(test_reminder_timeout);
    TH_RUN(test_reminder_only_once_per_cycle);
    TH_RUN(test_no_reminder_phone_placed_after_ignition_off);
}
