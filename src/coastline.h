#pragma once
// World coastline background for the radar scope.
// Project the embedded Natural Earth coastline (coastline_data.h) into screen
// polylines for the current scope, then draw them under the aircraft. Projection
// is done once per home/range change (cheap bbox cull + great-circle), never per
// frame; drawing happens inside the static chrome layer's DRAW_MAIN callback.
#include <lvgl.h>

// rotationDeg: the bearing at the top of the scope (0 = north-up), as geo::projectToScreen.
void coastline_project(double homeLat, double homeLon, double rangeKm,
                       float cx, float cy, float rOuterPx, double rotationDeg = 0.0);

void coastline_draw(lv_draw_ctx_t *ctx, lv_color_t color, lv_opa_t opa, lv_coord_t width);
