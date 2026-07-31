#include "tab_display.h"
#include <stdio.h>
#include <stdlib.h>

typedef enum {
    PATTERN_RED = 0,
    PATTERN_GREEN,
    PATTERN_BLUE,
    PATTERN_WHITE,
    PATTERN_BLACK,
    PATTERN_GRAYSCALE,
    PATTERN_GRID,
    PATTERN_EXERCISER,
    PATTERN_COUNT
} DisplayPattern;

static DisplayPattern current_pattern = PATTERN_RED;
static int exerciser_active = 0;
static int exerciser_frame = 0;
static int exerciser_speed = 1;

void tab_display_init(void) {
    current_pattern = PATTERN_RED;
    exerciser_active = 0;
    exerciser_frame = 0;
}

void tab_display_draw(GuiState *state, const SceCtrlData *pad) {
    if (!state) return;

    if (exerciser_active || current_pattern == PATTERN_EXERCISER) {
        exerciser_frame++;
        int cycle = (exerciser_frame / (exerciser_speed * 4)) % 3;
        unsigned int col = (cycle == 0) ? RGBA8(255, 0, 0, 255) :
                          ((cycle == 1) ? RGBA8(0, 255, 0, 255) : RGBA8(0, 0, 255, 255));
        
        vita2d_draw_rectangle(0, 0, 960, 544, col);

        if (state->font && (exerciser_frame % 120 < 60)) {
            vita2d_pgf_draw_text(state->font, 300, 500, RGBA8(255, 255, 255, 180), 1.0f, "[ OLED EXERCISER RUNNING - PRESS X TO EXIT ]");
        }
        return;
    }

    switch (current_pattern) {
        case PATTERN_RED:
            vita2d_draw_rectangle(0, 0, 960, 544, RGBA8(255, 0, 0, 255));
            break;
        case PATTERN_GREEN:
            vita2d_draw_rectangle(0, 0, 960, 544, RGBA8(0, 255, 0, 255));
            break;
        case PATTERN_BLUE:
            vita2d_draw_rectangle(0, 0, 960, 544, RGBA8(0, 0, 255, 255));
            break;
        case PATTERN_WHITE:
            vita2d_draw_rectangle(0, 0, 960, 544, RGBA8(255, 255, 255, 255));
            break;
        case PATTERN_BLACK:
            vita2d_draw_rectangle(0, 0, 960, 544, RGBA8(0, 0, 0, 255));
            break;
        case PATTERN_GRAYSCALE:
            for (int i = 0; i < 16; i++) {
                unsigned char val = (i * 255) / 15;
                vita2d_draw_rectangle(i * 60, 0, 60, 544, RGBA8(val, val, val, 255));
            }
            break;
        case PATTERN_GRID:
            vita2d_draw_rectangle(0, 0, 960, 544, RGBA8(15, 23, 42, 255));
            for (int x = 0; x < 960; x += 40) {
                vita2d_draw_line(x, 0, x, 544, RGBA8(100, 116, 139, 255));
            }
            for (int y = 0; y < 544; y += 40) {
                vita2d_draw_line(0, y, 960, y, RGBA8(100, 116, 139, 255));
            }
            break;
        default:
            break;
    }

    gui_draw_panel(40, 100, 880, 80, RGBA8(15, 23, 42, 210), COLOR_PANEL_BORDER);
    if (state->font) {
        vita2d_pgf_draw_text(state->font, 60, 130, COLOR_PRIMARY, 1.0f, "OLED & DISPLAY TEST / BURN-IN FIXER");
        
        char buf[128];
        static const char *names[] = {"PURE RED", "PURE GREEN", "PURE BLUE", "PURE WHITE", "PURE BLACK", "GRAYSCALE RAMP", "GRID PATTERN"};
        snprintf(buf, sizeof(buf), "Current Pattern: %s  |  D-Pad Left/Right to Switch  |  Square: Start OLED Exerciser", names[current_pattern]);
        vita2d_pgf_draw_text(state->font, 60, 160, COLOR_TEXT, 0.85f, buf);
    }
}

void tab_display_handle_input(GuiState *state, uint32_t pressed_buttons) {
    if (!state) return;

    if (exerciser_active) {
        if (pressed_buttons & (SCE_CTRL_CROSS | SCE_CTRL_CIRCLE | SCE_CTRL_SQUARE | SCE_CTRL_TRIANGLE)) {
            exerciser_active = 0;
        }
        return;
    }

    if (pressed_buttons & SCE_CTRL_RIGHT) {
        current_pattern = (current_pattern + 1) % (PATTERN_GRID + 1);
    } else if (pressed_buttons & SCE_CTRL_LEFT) {
        if (current_pattern == 0) current_pattern = PATTERN_GRID;
        else current_pattern--;
    }

    if (pressed_buttons & SCE_CTRL_SQUARE) {
        exerciser_active = 1;
        exerciser_frame = 0;
    }
}
