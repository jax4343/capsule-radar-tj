#include "airports.h"
#include "airports_data.h"
#include "geo.h"
#include <vector>
#include <math.h>
#include <string.h>
#include <stdio.h>

struct Apt { lv_point_t pos; char iata[4]; char icao[5]; uint8_t large; uint8_t pinned; };
static std::vector<Apt> s_apts;
static std::vector<int> s_pinIdx;    // table indices of the pinned airports

int airports_set_pinned(const AirportPins &pins, char *missing, size_t missingLen) {
    s_pinIdx.clear();
    if (missing && missingLen) missing[0] = 0;
    size_t w = 0;
    for (int k = 0; k < pins.count; ++k) {
        const char *code = pins.code[k];
        const size_t len = strlen(code);
        int found = -1;
        if (len == 3) {                                  // IATA first, then an ICAO-style id, then FAA "PAO" = "KPAO"
            for (int i = 0; i < AIRPORT_NUM && found < 0; ++i) if (strcmp(AIRPORT_IATA[i], code) == 0) found = i;
            for (int i = 0; i < AIRPORT_NUM && found < 0; ++i) if (strcmp(AIRPORT_ICAO[i], code) == 0) found = i;
            char faa[5] = { 'K', code[0], code[1], code[2], 0 };
            for (int i = 0; i < AIRPORT_NUM && found < 0; ++i) if (strcmp(AIRPORT_ICAO[i], faa) == 0) found = i;
        } else {
            for (int i = 0; i < AIRPORT_NUM && found < 0; ++i) if (strcmp(AIRPORT_ICAO[i], code) == 0) found = i;
        }
        if (found >= 0) { s_pinIdx.push_back(found); continue; }
        if (missing && w + strlen(pins.code[k]) + 2 <= missingLen) {
            w += snprintf(missing + w, missingLen - w, "%s%s", w ? " " : "", pins.code[k]);
        }
    }
    return (int)s_pinIdx.size();
}

static bool is_pinned(int i) {
    for (int p : s_pinIdx) if (p == i) return true;
    return false;
}

void airports_project(double homeLat, double homeLon, double rangeKm,
                      float cx, float cy, float rOuterPx, double rotationDeg) {
    s_apts.clear();
    if (rangeKm <= 0) return;

    const double rangeDeg  = rangeKm / 111.0;
    const double latMargin = rangeDeg * 1.05;
    const double cosLat    = cos(homeLat * M_PI / 180.0);
    const double lonMargin = latMargin / (cosLat < 0.15 ? 0.15 : cosLat);

    for (int i = 0; i < AIRPORT_NUM; ++i) {
        const double lat = AIRPORT_LAT[i] / (double)AIRPORT_SCALE;
        const double lon = AIRPORT_LON[i] / (double)AIRPORT_SCALE;
        const double dlon = lon - homeLon;
        if (AIRPORT_KIND[i] == 2 && !is_pinned(i)) continue;                // pin-only: shown only when pinned
        if (fabs(lat - homeLat) > latMargin) continue;                      // cheap bbox reject
        if (fabs(dlon) > lonMargin && fabs(fabs(dlon) - 360.0) > lonMargin) continue;
        const double dist = geo::haversineKm(homeLat, homeLon, lat, lon);
        if (dist > rangeKm) continue;                                       // only inside the scope
        const double brg = geo::bearingDeg(homeLat, homeLon, lat, lon);
        const double rPx = (dist / rangeKm) * rOuterPx;
        const double a   = (brg - rotationDeg) * M_PI / 180.0;
        Apt ap;
        ap.pos.x = (lv_coord_t)lroundf((float)(cx + rPx * sin(a)));
        ap.pos.y = (lv_coord_t)lroundf((float)(cy - rPx * cos(a)));
        memcpy(ap.iata, AIRPORT_IATA[i], 4);
        memcpy(ap.icao, AIRPORT_ICAO[i], 5);
        ap.large = AIRPORT_KIND[i] == 1;
        ap.pinned = is_pinned(i) ? 1 : 0;
        s_apts.push_back(ap);
    }
}

void airports_draw(lv_draw_ctx_t *ctx, lv_color_t color, lv_opa_t opa,
                   bool background, lv_color_t pinColor) {
    if (s_apts.empty()) return;

    lv_draw_arc_dsc_t ring;
    lv_draw_arc_dsc_init(&ring);
    ring.color = color; ring.width = 2; ring.opa = opa;

    lv_draw_rect_dsc_t dot;
    lv_draw_rect_dsc_init(&dot);
    dot.bg_color = color; dot.bg_opa = opa; dot.radius = LV_RADIUS_CIRCLE;

    lv_draw_label_dsc_t lbl;
    lv_draw_label_dsc_init(&lbl);
    lbl.color = color; lbl.opa = opa; lbl.font = &lv_font_montserrat_12;

    if (background) {
        for (const Apt &ap : s_apts) {
            if (ap.pinned) continue;                                        // drawn below, on top
            if (ap.large) {
                lv_draw_arc(ctx, &ring, &ap.pos, 3, 0, 360);                // small hollow ring
                if (ap.iata[0]) {
                    lv_area_t la = { (lv_coord_t)(ap.pos.x + 5), (lv_coord_t)(ap.pos.y - 7),
                                     (lv_coord_t)(ap.pos.x + 44), (lv_coord_t)(ap.pos.y + 7) };
                    lv_draw_label(ctx, &lbl, &la, ap.iata, NULL);
                }
            } else {
                lv_area_t d = { (lv_coord_t)(ap.pos.x - 1), (lv_coord_t)(ap.pos.y - 1),
                                (lv_coord_t)(ap.pos.x + 1), (lv_coord_t)(ap.pos.y + 1) };
                lv_draw_rect(ctx, &dot, &d);                                // faint dot
            }
        }
    }

    // pinned: a brighter, larger ring around a solid dot, and always a label
    ring.color = pinColor; ring.opa = LV_OPA_COVER;
    dot.bg_color = pinColor; dot.bg_opa = LV_OPA_COVER;
    lbl.color = pinColor; lbl.opa = LV_OPA_COVER; lbl.font = &lv_font_montserrat_14;
    for (const Apt &ap : s_apts) {
        if (!ap.pinned) continue;
        lv_draw_arc(ctx, &ring, &ap.pos, 6, 0, 360);
        lv_area_t d = { (lv_coord_t)(ap.pos.x - 2), (lv_coord_t)(ap.pos.y - 2),
                        (lv_coord_t)(ap.pos.x + 2), (lv_coord_t)(ap.pos.y + 2) };
        lv_draw_rect(ctx, &dot, &d);
        lv_area_t la = { (lv_coord_t)(ap.pos.x + 9), (lv_coord_t)(ap.pos.y - 8),
                         (lv_coord_t)(ap.pos.x + 52), (lv_coord_t)(ap.pos.y + 8) };
        lv_draw_label(ctx, &lbl, &la, ap.iata[0] ? ap.iata : ap.icao, NULL);
    }
}

bool airports_nearest_iata(double lat, double lon, float maxKm,
                           char iata[4], float *distKm, float *bearingDeg) {
    if (iata) iata[0] = 0;
    double best = maxKm;
    int bestIdx = -1;
    for (int i = 0; i < AIRPORT_NUM; ++i) {
        if (!AIRPORT_IATA[i][0]) continue;
        const double alat = AIRPORT_LAT[i] / (double)AIRPORT_SCALE;
        const double alon = AIRPORT_LON[i] / (double)AIRPORT_SCALE;
        const double d = geo::haversineKm(lat, lon, alat, alon);
        if (d < best) { best = d; bestIdx = i; }
    }
    if (bestIdx < 0) return false;
    if (iata) memcpy(iata, AIRPORT_IATA[bestIdx], 4);
    if (distKm) *distKm = (float)best;
    if (bearingDeg) {
        const double alat = AIRPORT_LAT[bestIdx] / (double)AIRPORT_SCALE;
        const double alon = AIRPORT_LON[bestIdx] / (double)AIRPORT_SCALE;
        *bearingDeg = (float)geo::bearingDeg(lat, lon, alat, alon);
    }
    return true;
}
