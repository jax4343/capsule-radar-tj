#pragma once
// Airport markers for the radar scope. Projects the embedded OurAirports list
// (airports_data.h) like the coastline: cull to the scope, great-circle project,
// cache screen markers, draw in the static chrome layer. Large airports get a small
// ring + IATA label; medium airports are a faint dot. Projection is done only on a
// home/range change, never per frame.
#include <lvgl.h>
#include <stddef.h>
#include "airport_pins.h"

// rotationDeg: the bearing at the top of the scope (0 = north-up), as geo::projectToScreen.
void airports_project(double homeLat, double homeLon, double rangeKm,
                      float cx, float cy, float rOuterPx, double rotationDeg = 0.0);

// Airports the user pinned on the config page. Matched against the embedded table by IATA
// or ICAO code (a 3-letter US FAA id such as PAO also finds KPAO); a pinned airport is always drawn with its label, whatever its size. Returns how
// many matched; the codes that did not are written to `missing` (space-separated, may be
// null). Call airports_project() afterwards to rebuild the markers.
int airports_set_pinned(const AirportPins &pins, char *missing, size_t missingLen);

// background: draw the table's own markers (the "Show airports" switch); pinned airports
// are drawn either way, in pinColor.
void airports_draw(lv_draw_ctx_t *ctx, lv_color_t color, lv_opa_t opa,
                   bool background, lv_color_t pinColor);

// Find the nearest recognizable airport (one with an IATA code). Used by the
// weather view for aviation context; entirely offline from the embedded data.
bool airports_nearest_iata(double lat, double lon, float maxKm,
                           char iata[4], float *distKm, float *bearingDeg);
