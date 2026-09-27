#include "th.h"

#include <stdio.h>

void run_fod_detector_tests(void);
void run_thermal_manager_tests(void);
void run_nfc_guard_tests(void);
void run_can_status_tests(void);
void run_phone_reminder_tests(void);

int main(int argc, char **argv)
{
    const char *junit = (argc > 1) ? argv[1] : "build/junit.xml";

    run_fod_detector_tests();
    run_thermal_manager_tests();
    run_nfc_guard_tests();
    run_can_status_tests();
    run_phone_reminder_tests();

    return th_finish(junit) == 0 ? 0 : 1;
}
