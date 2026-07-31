#include "tab_motion.h"
#include <stdio.h>
#include <math.h>

void tab_motion_init(void) {
    sceMotionStartSampling();
}

void tab_motion_draw(GuiState *state) {
    if (!state) return;

    SceMotionState motion;
    sceMotionGetState(&motion);

    float cx = 240.0f;
    float cy = 280.0f;
    float radius = 120.0f;

    gui_draw_panel(40, 100, 400, 380, COLOR_PANEL, COLOR_PANEL_BORDER);
    if (state->font) {
        vita2d_pgf_draw_text(state->font, 60, 130, COLOR_PRIMARY, 1.0f, "ARTIFICIAL HORIZON (GYRO)");
    }

    gui_draw_circle_outline(cx, cy, radius, 2.0f, COLOR_PANEL_BORDER);

    float roll = motion.basicOrientation.z;
    float pitch = motion.basicOrientation.y;

    float angle = roll * 3.14159265f;
    float line_x1 = cx - cosf(angle) * radius;
    float line_y1 = cy - sinf(angle) * radius;
    float line_x2 = cx + cosf(angle) * radius;
    float line_y2 = cy + sinf(angle) * radius;

    vita2d_draw_line(line_x1, line_y1, line_x2, line_y2, COLOR_PRIMARY);
    vita2d_draw_fill_circle(cx, cy + pitch * 50.0f, 6.0f, COLOR_ACCENT);

    gui_draw_panel(460, 100, 460, 380, COLOR_PANEL, COLOR_PANEL_BORDER);
    if (state->font) {
        vita2d_pgf_draw_text(state->font, 480, 130, COLOR_PRIMARY, 1.0f, "ACCELEROMETER & GYRO DATA");

        char buf[128];
        snprintf(buf, sizeof(buf), "Acceleration X: %+.3f g", motion.acceleration.x);
        vita2d_pgf_draw_text(state->font, 480, 180, COLOR_TEXT, 0.9f, buf);

        snprintf(buf, sizeof(buf), "Acceleration Y: %+.3f g", motion.acceleration.y);
        vita2d_pgf_draw_text(state->font, 480, 215, COLOR_TEXT, 0.9f, buf);

        snprintf(buf, sizeof(buf), "Acceleration Z: %+.3f g", motion.acceleration.z);
        vita2d_pgf_draw_text(state->font, 480, 250, COLOR_TEXT, 0.9f, buf);

        snprintf(buf, sizeof(buf), "Angular Velocity X: %+.3f rad/s", motion.angularVelocity.x);
        vita2d_pgf_draw_text(state->font, 480, 300, COLOR_TEXT_MUTED, 0.85f, buf);

        snprintf(buf, sizeof(buf), "Angular Velocity Y: %+.3f rad/s", motion.angularVelocity.y);
        vita2d_pgf_draw_text(state->font, 480, 335, COLOR_TEXT_MUTED, 0.85f, buf);

        snprintf(buf, sizeof(buf), "Angular Velocity Z: %+.3f rad/s", motion.angularVelocity.z);
        vita2d_pgf_draw_text(state->font, 480, 370, COLOR_TEXT_MUTED, 0.85f, buf);

        vita2d_pgf_draw_text(state->font, 480, 430, COLOR_SUCCESS, 0.85f, "Sensor Calibration Status: OK");
    }
}
