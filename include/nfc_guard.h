/**
 * @file nfc_guard.h
 * @brief NFC / RFID key-card protection.
 *
 * The WCM's NFC reader polls the charging surface between power-transfer
 * frames. If a key card (or any NFC/RFID tag) is detected, power transfer is
 * paused immediately to avoid damaging the card. Power resumes only after the
 * card has been absent for NFC_GUARD_RESUME_POLLS consecutive polls.
 */
#ifndef NFC_GUARD_H
#define NFC_GUARD_H

#include <stdint.h>
#include <stdbool.h>

#define NFC_GUARD_RESUME_POLLS 3u

typedef enum {
    NFC_GUARD_CLEAR = 0,   /**< no card, power transfer allowed */
    NFC_GUARD_PAUSED       /**< card present or recently seen */
} nfc_guard_state_t;

typedef struct {
    nfc_guard_state_t state;
    uint8_t           absent_polls;
    uint16_t          pause_events;   /**< diagnostic counter */
} nfc_guard_t;

void nfc_guard_init(nfc_guard_t *g);
/** Feed one NFC poll result. Returns true if power transfer is allowed. */
bool nfc_guard_update(nfc_guard_t *g, bool card_present);
bool nfc_guard_power_allowed(const nfc_guard_t *g);

#endif /* NFC_GUARD_H */
