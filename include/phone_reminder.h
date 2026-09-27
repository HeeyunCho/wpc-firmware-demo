/**
 * @file phone_reminder.h
 * @brief Phone-left-behind reminder.
 *
 * When ignition goes ON -> OFF while a phone is on the charging pad, the
 * reminder is armed. If the phone is still on the pad when the driver door
 * opens within PHONE_REMINDER_TIMEOUT_MS, a warning is requested (cluster
 * message + chime). The warning is issued at most once per ignition cycle and
 * is cancelled when the phone is removed.
 */
#ifndef PHONE_REMINDER_H
#define PHONE_REMINDER_H

#include <stdint.h>
#include <stdbool.h>

#define PHONE_REMINDER_TIMEOUT_MS 120000u

typedef enum {
    PHONE_REMINDER_IDLE = 0,
    PHONE_REMINDER_ARMED,
    PHONE_REMINDER_WARNING,
    PHONE_REMINDER_DONE
} phone_reminder_state_t;

typedef struct {
    bool     ignition_on;
    bool     phone_present;
    bool     driver_door_open;
    uint32_t now_ms;
} phone_reminder_inputs_t;

typedef struct {
    phone_reminder_state_t state;
    bool                   prev_ignition_on;
    uint32_t               armed_at_ms;
} phone_reminder_t;

void phone_reminder_init(phone_reminder_t *r);
/** Returns true while the warning should be displayed. */
bool phone_reminder_update(phone_reminder_t *r, const phone_reminder_inputs_t *in);

#endif /* PHONE_REMINDER_H */
