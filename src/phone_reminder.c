#include "phone_reminder.h"

#include <stddef.h>

void phone_reminder_init(phone_reminder_t *r)
{
    if (r == NULL) {
        return;
    }
    r->state = PHONE_REMINDER_IDLE;
    r->prev_ignition_on = false;
    r->armed_at_ms = 0u;
}

bool phone_reminder_update(phone_reminder_t *r, const phone_reminder_inputs_t *in)
{
    bool ign_off_edge;

    if (r == NULL || in == NULL) {
        return false;
    }

    ign_off_edge = r->prev_ignition_on && !in->ignition_on;
    r->prev_ignition_on = in->ignition_on;

    if (in->ignition_on) {
        /* New drive cycle: reset. */
        r->state = PHONE_REMINDER_IDLE;
        return false;
    }

    switch (r->state) {
    case PHONE_REMINDER_IDLE:
        if (ign_off_edge && in->phone_present) {
            r->state = PHONE_REMINDER_ARMED;
            r->armed_at_ms = in->now_ms;
        }
        break;

    case PHONE_REMINDER_ARMED:
        if (!in->phone_present) {
            r->state = PHONE_REMINDER_DONE;
        } else if ((uint32_t)(in->now_ms - r->armed_at_ms) > PHONE_REMINDER_TIMEOUT_MS) {
            r->state = PHONE_REMINDER_DONE;
        } else if (in->driver_door_open) {
            r->state = PHONE_REMINDER_WARNING;
        }
        break;

    case PHONE_REMINDER_WARNING:
        if (!in->phone_present || !in->driver_door_open) {
            r->state = PHONE_REMINDER_DONE;
        }
        break;

    case PHONE_REMINDER_DONE:
    default:
        break;
    }

    return r->state == PHONE_REMINDER_WARNING;
}
