// Pinned-airport list parsing (src/airport_pins.h): what the config page's text box may hold.
#include "airport_pins.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static AirportPins P(const char *s) { return parseAirportPins(s); }

int main() {
    // --- accepted: separators, case, ICAO-length codes ---
    AirportPins a = P("ALC,VLC");
    assert(a.count == 2 && a.rejected == 0);
    assert(!strcmp(a.code[0], "ALC") && !strcmp(a.code[1], "VLC"));

    a = P("  alc ; vlc\tmad\n");                 // lower case, mixed separators, stray whitespace
    assert(a.count == 3 && a.rejected == 0);
    assert(!strcmp(a.code[0], "ALC") && !strcmp(a.code[1], "VLC") && !strcmp(a.code[2], "MAD"));

    a = P("LEAL");                              // 4 characters: an ICAO code is kept
    assert(a.count == 1 && !strcmp(a.code[0], "LEAL"));

    a = P("alc,ALC, Alc");                      // repeats collapse and are not an error
    assert(a.count == 1 && a.rejected == 0);

    // --- empty / null ---
    a = P("");
    assert(a.count == 0 && a.rejected == 0);
    a = P(" ,; ");
    assert(a.count == 0 && a.rejected == 0);
    a = parseAirportPins(nullptr);
    assert(a.count == 0 && a.rejected == 0);

    // --- rejected: counted, never turned into a different airport ---
    a = P("AL");                                // too short
    assert(a.count == 0 && a.rejected == 1);
    a = P("ABCDE");                             // too long
    assert(a.count == 0 && a.rejected == 1);
    a = P("AL-C");                              // a character that is not a letter or digit
    assert(a.count == 0 && a.rejected == 1);
    a = P("A!B,VLC");                           // one bad entry does not spoil the next
    assert(a.count == 1 && !strcmp(a.code[0], "VLC") && a.rejected == 1);
    a = P("ABCDEFGHIJKLMNOPQRSTUVWXYZ");        // longer than the token buffer
    assert(a.count == 0 && a.rejected == 1);

    // --- the cap ---
    a = P("AAA BBB CCC DDD EEE FFF GGG HHH III JJJ");
    assert(a.count == AIRPORT_PIN_MAX && a.rejected == 2);
    assert(!strcmp(a.code[AIRPORT_PIN_MAX - 1], "HHH"));

    // --- join: the NVS / page form ---
    char out[AIRPORT_PIN_MAX * (AIRPORT_PIN_CODE + 1) + 1];
    joinAirportPins(P("alc vlc"), out, sizeof(out));
    assert(!strcmp(out, "ALC,VLC"));
    joinAirportPins(P(""), out, sizeof(out));
    assert(out[0] == '\0');
    joinAirportPins(P("AAAA BBBB CCCC DDDD EEEE FFFF GGGG HHHH"), out, sizeof(out));
    assert(strlen(out) == AIRPORT_PIN_MAX * (AIRPORT_PIN_CODE + 1) - 1);   // worst case fits exactly

    // too small a buffer: truncate on a whole code, still terminated
    char tiny[6];
    joinAirportPins(P("ALC VLC"), tiny, sizeof(tiny));
    assert(!strcmp(tiny, "ALC"));

    // a joined list parses back to the same list (what loadSettings does with NVS)
    joinAirportPins(P("alc, vlc ,mad"), out, sizeof(out));
    a = P(out);
    assert(a.count == 3 && a.rejected == 0 && !strcmp(a.code[2], "MAD"));

    puts("airport_pins: OK");
    return 0;
}
