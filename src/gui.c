#include "gui.h"
#include <math.h>
#include <string.h>

static const char *tab_names[TAB_COUNT] = {
    "[1] STICKS",
    "[2] BATTERY",
    "[3] DISPLAY",
    "[4] MOTION",
    "[5] AUDIO"
};

void gui_init(GuiState *state) {
    if (!state) return;
    state->current_tab = TAB_ANALOG;
    state->screen_width = 960;
    state->screen_height = 544;
    state->show_disclaimer = 1;
    state->font = vita2d_load_default_pgf();
    config_load(&state->config);
}

void gui_finish(GuiState *state) {
    if (!state) return;
    if (state->font) {
        vita2d_free_pgf(state->font);
        state->font = NULL;
    }
}

void gui_draw_panel(float x, float y, float w, float h, unsigned int bg_color, unsigned int border_color) {
    vita2d_draw_rectangle(x, y, w, h, bg_color);
    if (border_color != 0) {
        vita2d_draw_line(x, y, x + w, y, border_color);
        vita2d_draw_line(x + w, y, x + w, y + h, border_color);
        vita2d_draw_line(x + w, y + h, x, y + h, border_color);
        vita2d_draw_line(x, y + h, x, y, border_color);
    }
}

void gui_draw_header(GuiState *state, const char *title) {
    vita2d_draw_rectangle(0, 0, 960, 48, COLOR_PANEL);
    vita2d_draw_line(0, 48, 960, 48, COLOR_PANEL_BORDER);

    if (state->font) {
        vita2d_pgf_draw_text(state->font, 20, 32, COLOR_PRIMARY, 1.3f, "VitaDoctor");
        vita2d_pgf_draw_text(state->font, 160, 31, COLOR_TEXT_MUTED, 1.0f, "|");
        if (title) {
            vita2d_pgf_draw_text(state->font, 180, 31, COLOR_TEXT, 1.0f, title);
        }
        vita2d_pgf_draw_text(state->font, 840, 31, COLOR_TEXT_MUTED, 0.85f, "v1.00");
    }
}

void gui_draw_tabs(GuiState *state) {
    float tab_w = 960.0f / TAB_COUNT;
    float tab_h = 36.0f;
    float y = 48.0f;

    vita2d_draw_rectangle(0, y, 960, tab_h, COLOR_PANEL);
    vita2d_draw_line(0, y + tab_h, 960, y + tab_h, COLOR_PRIMARY);

    for (int i = 0; i < TAB_COUNT; i++) {
        float x = i * tab_w;
        int active = (state->current_tab == (GuiTab)i);

        if (active) {
            vita2d_draw_rectangle(x, y, tab_w, tab_h, COLOR_PRIMARY);
        } else {
            vita2d_draw_line(x + tab_w, y + 6, x + tab_w, y + tab_h - 6, COLOR_PANEL_BORDER);
        }

        if (state->font) {
            unsigned int color = active ? COLOR_BG : COLOR_TEXT_MUTED;
            float txt_x = x + (tab_w / 2.0f) - (strlen(tab_names[i]) * 4.2f);
            vita2d_pgf_draw_text(state->font, txt_x, y + 24, color, 0.85f, tab_names[i]);
        }
    }
}

void gui_draw_circle_outline(float x, float y, float radius, float stroke_width, unsigned int color) {
    int segments = 48;
    float step = 2.0f * 3.14159265f / segments;

    for (int i = 0; i < segments; i++) {
        float a1 = i * step;
        float a2 = (i + 1) * step;

        float x1 = x + cosf(a1) * radius;
        float y1 = y + sinf(a1) * radius;
        float x2 = x + cosf(a2) * radius;
        float y2 = y + sinf(a2) * radius;

        vita2d_draw_line(x1, y1, x2, y2, color);
    }
}

void gui_draw_progress_bar(float x, float y, float w, float h, float progress, unsigned int fill_color, unsigned int bg_color) {
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;

    vita2d_draw_rectangle(x, y, w, h, bg_color);
    vita2d_draw_rectangle(x, y, w * progress, h, fill_color);
    vita2d_draw_line(x, y, x + w, y, COLOR_PANEL_BORDER);
    vita2d_draw_line(x + w, y, x + w, y + h, COLOR_PANEL_BORDER);
    vita2d_draw_line(x + w, y + h, x, y + h, COLOR_PANEL_BORDER);
    vita2d_draw_line(x, y + h, x, y, COLOR_PANEL_BORDER);
}

void gui_draw_disclaimer(GuiState *state) {
    if (!state || !state->show_disclaimer) return;

    vita2d_draw_rectangle(0, 0, 960, 544, RGBA8(0, 0, 0, 200));

    float x = 130.0f;
    float y = 95.0f;
    float w = 700.0f;
    float h = 350.0f;

    gui_draw_panel(x, y, w, h, COLOR_PANEL, COLOR_WARNING);

    if (state->font) {
        vita2d_pgf_draw_text(state->font, x + 30, y + 42, COLOR_WARNING, 1.25f, "NOTICE & DISCLAIMER");
        vita2d_draw_line(x + 30, y + 58, x + w - 30, y + 58, COLOR_PANEL_BORDER);

        vita2d_pgf_draw_text(state->font, x + 30, y + 95, COLOR_TEXT, 0.88f,
            "VitaDoctor is a hardware diagnostic and testing suite for PS Vita.");

        vita2d_pgf_draw_text(state->font, x + 30, y + 135, COLOR_TEXT_MUTED, 0.82f,
            "- Battery health, voltage and temperature are estimations retrieved");
        vita2d_pgf_draw_text(state->font, x + 42, y + 157, COLOR_TEXT_MUTED, 0.82f,
            "from system hardware registers (Syscon/PMIC).");

        vita2d_pgf_draw_text(state->font, x + 30, y + 190, COLOR_TEXT_MUTED, 0.82f,
            "- Analog stick movement extending past the circle on diagonals is normal.");

        vita2d_pgf_draw_text(state->font, x + 30, y + 225, COLOR_TEXT_MUTED, 0.82f,
            "- This application is designed purely for hardware testing & diagnostics.");

        gui_draw_panel(x + 200, y + 275, 300, 42, COLOR_PRIMARY, COLOR_PRIMARY);
        vita2d_pgf_draw_text(state->font, x + 225, y + 302, COLOR_BG, 0.95f, "Press CROSS (X) to Continue");
    }
}
