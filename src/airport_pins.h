#pragma once
// The "pinned airports" list typed on the config page: parse, normalise, cap.
//
// Pure (no Arduino, no LVGL) so tests/airport_pins_test.cpp can exercise it on the host.
// Anything that is not a plausible identifier is counted in `rejected` and dropped; the
// web page reports it, so a typo never silently becomes a different airport.
#include <stddef.h>
#include <string.h>

#define AIRPORT_PIN_MAX  8     // pinned airports kept; each one gets a label on the scope
#define AIRPORT_PIN_CODE 4     // longest identifier kept (an ICAO code)
#define AIRPORT_PIN_MIN  3     // shortest (an IATA code)

struct AirportPins {
    char code[AIRPORT_PIN_MAX][AIRPORT_PIN_CODE + 1];
    int  count;
    int  rejected;   // entries dropped: wrong length or characters, or past the cap (duplicates are not counted)
};

// Split on commas, semicolons and whitespace; upper-case; keep 3..4 letters/digits each.
inline AirportPins parseAirportPins(const char *s) {
    AirportPins out;
    out.count = 0;
    out.rejected = 0;
    if (!s) return out;

    char tok[16];
    size_t n = 0;
    bool bad = false;
    for (const char *p = s;; ++p) {
        const char c = *p;
        const bool sep = c == '\0' || c == ',' || c == ';' || c == ' ' || c == '\t' || c == '\n' || c == '\r';
        if (!sep) {
            const bool lower = c >= 'a' && c <= 'z';
            const bool alnum = lower || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
            if (!alnum || n >= sizeof(tok) - 1) bad = true;
            else tok[n++] = lower ? (char)(c - 'a' + 'A') : c;
            continue;
        }
        if (n || bad) {                                   // a token just ended
            tok[n] = '\0';
            bool dup = false;
            for (int i = 0; i < out.count; ++i) if (strcmp(out.code[i], tok) == 0) dup = true;
            if (bad || n < AIRPORT_PIN_MIN || n > AIRPORT_PIN_CODE) ++out.rejected;
            else if (dup) { /* repeated: harmless, not an error */ }
            else if (out.count >= AIRPORT_PIN_MAX) ++out.rejected;
            else { memcpy(out.code[out.count], tok, n + 1); ++out.count; }
        }
        n = 0;
        bad = false;
        if (c == '\0') break;
    }
    return out;
}

// "ALC,VLC" — the form stored in NVS and shown back in the page. Only A-Z, 0-9 and commas,
// so it is safe to put straight into an HTML attribute.
inline void joinAirportPins(const AirportPins &pins, char *dst, size_t dstLen) {
    if (!dst || !dstLen) return;
    size_t w = 0;
    dst[0] = '\0';
    for (int i = 0; i < pins.count; ++i) {
        const size_t len = strlen(pins.code[i]);
        if (w + len + 2 > dstLen) break;
        if (i) dst[w++] = ',';
        memcpy(dst + w, pins.code[i], len);
        w += len;
        dst[w] = '\0';
    }
}
