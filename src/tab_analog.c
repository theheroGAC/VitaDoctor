#include "tab_analog.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void draw_btn_badge(vita2d_pgf *font, float x, float y, float w, float h, const char *label, int is_pressed) {
    unsigned int bg = is_pressed ? COLOR_SUCCESS : COLOR_PANEL;
    unsigned int border = is_pressed ? COLOR_SUCCESS : COLOR_PANEL_BORDER;
    unsigned int txt_col = is_pressed ? COLOR_BG : COLOR_TEXT;

    gui_draw_panel(x, y, w, h, bg, border);
    if (font) {
        float txt_x = x + (w / 2.0f) - (strlen(label) * 3.5f);
        float txt_y = y + (h / 2.0f) + 5.0f;
        vita2d_pgf_draw_text(font, txt_x, txt_y, txt_col, 0.75f, label);
    }
}

static void draw_stick_widget(GuiState *state, float px, float py, float pw, float ph, float cx, float cy, float radius, int raw_x, int raw_y, const char *title) {
    gui_draw_panel(px, py, pw, ph, COLOR_PANEL, COLOR_PANEL_BORDER);

    if (state->font) {
        vita2d_pgf_draw_text(state->font, px + 15, py + 30, COLOR_PRIMARY, 0.95f, title);
    }

    gui_draw_circle_outline(cx, cy, radius, 1.5f, COLOR_PANEL_BORDER);
    vita2d_draw_line(cx - radius, cy, cx + radius, cy, COLOR_PANEL_BORDER);
    vita2d_draw_line(cx, cy - radius, cx, cy + radius, COLOR_PANEL_BORDER);

    float deadzone_r = radius * 0.11f;
    gui_draw_circle_outline(cx, cy, deadzone_r, 1.5f, COLOR_DANGER);

    int norm_x = raw_x - 128;
    int norm_y = raw_y - 128;

    float stick_px = cx + (norm_x / 128.0f) * radius;
    float stick_py = cy + (norm_y / 128.0f) * radius;

    float dist_from_center = sqrtf(norm_x * norm_x + norm_y * norm_y);
    unsigned int dot_color = (dist_from_center < (11.0f * 1.28f)) ? COLOR_DANGER : COLOR_SUCCESS;

    vita2d_draw_fill_circle(stick_px, stick_py, 6.0f, dot_color);
    vita2d_draw_line(cx, cy, stick_px, stick_py, dot_color);

    if (state->font) {
        char buf[64];
        snprintf(buf, sizeof(buf), "Raw X: %3d  Y: %3d", raw_x, raw_y);
        vita2d_pgf_draw_text(state->font, px + 15, py + ph - 35, COLOR_TEXT, 0.8f, buf);

        float pct_dist = (dist_from_center / 128.0f) * 100.0f;
        snprintf(buf, sizeof(buf), "Stick Offset: %d%%", (int)pct_dist);
        vita2d_pgf_draw_text(state->font, px + 15, py + ph - 15, COLOR_WARNING, 0.8f, buf);
    }
}

void tab_analog_draw(GuiState *state, const SceCtrlData *pad, const SceTouchData *touch_front, const SceTouchData *touch_back) {
    if (!state || !pad) return;

    draw_stick_widget(state, 40, 95, 300, 310, 190, 230, 75, pad->lx, pad->ly, "LEFT ANALOG");
    draw_stick_widget(state, 620, 95, 300, 310, 770, 230, 75, pad->rx, pad->ry, "RIGHT ANALOG");

    gui_draw_panel(360, 95, 240, 310, COLOR_PANEL, COLOR_PANEL_BORDER);
    if (state->font) {
        vita2d_pgf_draw_text(state->font, 375, 120, COLOR_PRIMARY, 0.9f, "BUTTON TEST MATRIX");
    }

    uint32_t btns = pad->buttons;

    draw_btn_badge(state->font, 375, 132, 45, 22, "L", (btns & (SCE_CTRL_LTRIGGER | SCE_CTRL_L1)));
    draw_btn_badge(state->font, 535, 132, 45, 22, "R", (btns & (SCE_CTRL_RTRIGGER | SCE_CTRL_R1)));

    draw_btn_badge(state->font, 405, 160, 30, 22, "U", (btns & SCE_CTRL_UP));
    draw_btn_badge(state->font, 375, 185, 30, 22, "L", (btns & SCE_CTRL_LEFT));
    draw_btn_badge(state->font, 435, 185, 30, 22, "R", (btns & SCE_CTRL_RIGHT));
    draw_btn_badge(state->font, 405, 210, 30, 22, "D", (btns & SCE_CTRL_DOWN));

    draw_btn_badge(state->font, 520, 160, 30, 22, "T", (btns & SCE_CTRL_TRIANGLE));
    draw_btn_badge(state->font, 490, 185, 30, 22, "S", (btns & SCE_CTRL_SQUARE));
    draw_btn_badge(state->font, 550, 185, 30, 22, "C", (btns & SCE_CTRL_CIRCLE));
    draw_btn_badge(state->font, 520, 210, 30, 22, "X", (btns & SCE_CTRL_CROSS));

    draw_btn_badge(state->font, 375, 240, 45, 22, "SEL", (btns & SCE_CTRL_SELECT));
    draw_btn_badge(state->font, 535, 240, 45, 22, "STA", (btns & SCE_CTRL_START));

    if (state->font) {
        vita2d_pgf_draw_text(state->font, 375, 290, COLOR_TEXT_MUTED, 0.78f, "Press buttons to test");
        vita2d_pgf_draw_text(state->font, 375, 320, COLOR_ACCENT, 0.78f, "SEL + L/R: Switch Tab");
        vita2d_pgf_draw_text(state->font, 375, 350, COLOR_DANGER, 0.78f, "SEL + START: Exit");
    }

    gui_draw_panel(40, 420, 880, 80, COLOR_PANEL, COLOR_PANEL_BORDER);
    if (state->font) {
        vita2d_pgf_draw_text(state->font, 55, 445, COLOR_PRIMARY, 0.85f, "TOUCH PANEL DIAGNOSTIC");
        
        char touch_info[128];
        int front_reports = touch_front ? touch_front->reportNum : 0;
        int back_reports = touch_back ? touch_back->reportNum : 0;
        snprintf(touch_info, sizeof(touch_info), "Front Touch Points: %d   |   Rear Touch Points: %d", front_reports, back_reports);
        vita2d_pgf_draw_text(state->font, 55, 475, COLOR_TEXT, 0.8f, touch_info);
    }

    if (touch_front) {
        for (int i = 0; i < touch_front->reportNum; i++) {
            float tx = (touch_front->report[i].x / 1920.0f) * 960.0f;
            float ty = (touch_front->report[i].y / 1088.0f) * 544.0f;
            vita2d_draw_fill_circle(tx, ty, 12.0f, COLOR_ACCENT);
        }
    }
}

void tab_analog_handle_input(GuiState *state, const SceCtrlData *pad, uint32_t pressed_buttons) {
    (void)state;
    (void)pad;
    (void)pressed_buttons;
}
