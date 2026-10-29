// Which compass bearing is at the top of the scope (src/compass.h + geo::projectToScreen).
#include "compass.h"
#include "geo.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

static bool near(float a, float b) { return fabsf(a - b) < 0.01f; }

int main() {
    // --- normalising whatever the page or NVS hands over ---
    assert(normalizeTopDeg(0) == 0);
    assert(normalizeTopDeg(90) == 90);
    assert(normalizeTopDeg(359) == 359);
    assert(normalizeTopDeg(360) == 0);
    assert(normalizeTopDeg(450) == 90);
    assert(normalizeTopDeg(-90) == 270);
    assert(normalizeTopDeg(-360) == 0);
    assert(normalizeTopDeg(-1) == 359);

    // --- the choices the page offers are 8 distinct, already-normal bearings ---
    assert(TOP_POINTS_N == 8);
    for (int i = 0; i < TOP_POINTS_N; ++i) {
        assert(TOP_POINTS[i].deg == normalizeTopDeg(TOP_POINTS[i].deg));
        assert(TOP_POINTS[i].deg == i * 45);
    }

    const float cx = 233, cy = 233, R = 218;

    // --- north at the top (default): a target due north is straight up, due east is right ---
    geo::Point p = geo::projectToScreen(10, 0, 20, cx, cy, R, 0);
    assert(near(p.x, cx) && p.y < cy);
    p = geo::projectToScreen(10, 90, 20, cx, cy, R, 0);
    assert(near(p.y, cy) && p.x > cx);

    // --- east at the top: a target due east is now straight up, due north is to the LEFT ---
    p = geo::projectToScreen(10, 90, 20, cx, cy, R, 90);
    assert(near(p.x, cx) && p.y < cy);
    p = geo::projectToScreen(10, 0, 20, cx, cy, R, 90);
    assert(near(p.y, cy) && p.x < cx);

    // --- south at the top: everything is upside down ---
    p = geo::projectToScreen(10, 180, 20, cx, cy, R, 180);
    assert(near(p.x, cx) && p.y < cy);
    p = geo::projectToScreen(10, 0, 20, cx, cy, R, 180);
    assert(near(p.x, cx) && p.y > cy);

    // --- the distance from the centre never changes with the turn ---
    const float d0 = hypotf(geo::projectToScreen(7, 33, 20, cx, cy, R, 0).x - cx,
                            geo::projectToScreen(7, 33, 20, cx, cy, R, 0).y - cy);
    for (int top = 0; top < 360; top += 45) {
        const geo::Point q = geo::projectToScreen(7, 33, 20, cx, cy, R, top);
        assert(near(hypotf(q.x - cx, q.y - cy), d0));
    }

    // --- screenBearingDeg agrees with the projection (glyphs and rim arrows use it) ---
    assert(near((float)screenBearingDeg(90, 90), 0.0f));     // the top bearing maps to straight up
    assert(near((float)screenBearingDeg(0, 90), -90.0f));    // north is 90 degrees counter-clockwise of up

    puts("compass: OK");
    return 0;
}
