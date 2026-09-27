#include "nfc_guard.h"

#include <stddef.h>

void nfc_guard_init(nfc_guard_t *g)
{
    if (g == NULL) {
        return;
    }
    g->state = NFC_GUARD_CLEAR;
    g->absent_polls = 0u;
    g->pause_events = 0u;
}

bool nfc_guard_update(nfc_guard_t *g, bool card_present)
{
    if (g == NULL) {
        return false; /* fail safe */
    }

    if (card_present) {
        if (g->state == NFC_GUARD_CLEAR) {
            g->pause_events++;
        }
        g->state = NFC_GUARD_PAUSED;
        g->absent_polls = 0u;
    } else if (g->state == NFC_GUARD_PAUSED) {
        if (g->absent_polls < 0xFFu) {
            g->absent_polls++;
        }
        if (g->absent_polls >= NFC_GUARD_RESUME_POLLS) {
            g->state = NFC_GUARD_CLEAR;
            g->absent_polls = 0u;
        }
    }
    return nfc_guard_power_allowed(g);
}

bool nfc_guard_power_allowed(const nfc_guard_t *g)
{
    return g != NULL && g->state == NFC_GUARD_CLEAR;
}
