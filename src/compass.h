#pragma once
// Which compass bearing sits at the top of the scope (the web page's "Top of screen").
//
// The value is a true bearing in whole degrees, 0 = north at the top (the default). It is
// what geo::projectToScreen takes as rotationDeg, so a point whose bearing equals the value
// is drawn straight up. Pure, so tests/compass_test.cpp runs on the host.
#include <stdlib.h>

struct CompassPoint { int deg; const char *name; };

// The choices the page offers, in order.
static const CompassPoint TOP_POINTS[] = {
    {0, "North"}, {45, "North-east"}, {90, "East"}, {135, "South-east"},
    {180, "South"}, {225, "South-west"}, {270, "West"}, {315, "North-west"},
};
static const int TOP_POINTS_N = (int)(sizeof(TOP_POINTS) / sizeof(TOP_POINTS[0]));

// Any integer -> 0..359 (-90 -> 270, 360 -> 0).
inline int normalizeTopDeg(long deg) {
    long d = deg % 360;
    if (d < 0) d += 360;
    return (int)d;
}

// Where a true bearing lands on the screen, in degrees clockwise from straight up.
inline double screenBearingDeg(double bearingDeg, double topDeg) { return bearingDeg - topDeg; }
