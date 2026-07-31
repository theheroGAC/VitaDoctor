#ifndef GUI_H
#define GUI_H

#include <vitasdk.h>
#include <vita2d.h>
#include "config.h"

#define COLOR_BG           RGBA8(15, 23, 42, 255)
#define COLOR_PANEL        RGBA8(30, 41, 59, 255)
#define COLOR_PANEL_BORDER RGBA8(51, 65, 85, 255)
#define COLOR_TEXT         RGBA8(241, 245, 249, 255)
#define COLOR_TEXT_MUTED   RGBA8(148, 163, 184, 255)
#define COLOR_PRIMARY      RGBA8(6, 182, 212, 255)
#define COLOR_ACCENT       RGBA8(139, 92, 246, 255)
#define COLOR_SUCCESS      RGBA8(16, 185, 129, 255)
#define COLOR_WARNING      RGBA8(245, 158, 11, 255)
#define COLOR_DANGER       RGBA8(244, 63, 94, 255)

typedef enum {
    TAB_ANALOG = 0,
    TAB_BATTERY,
    TAB_DISPLAY,
    TAB_MOTION,
    TAB_AUDIO,
    TAB_SYSTEM,
    TAB_COUNT
} GuiTab;

typedef struct {
    GuiTab current_tab;
    vita2d_pgf *font;
    VitaDoctorConfig config;
    int screen_width;
    int screen_height;
    int show_disclaimer;
} GuiState;

void gui_init(GuiState *state);
void gui_finish(GuiState *state);
void gui_draw_header(GuiState *state, const char *title);
void gui_draw_tabs(GuiState *state);
void gui_draw_panel(float x, float y, float w, float h, unsigned int bg_color, unsigned int border_color);
void gui_draw_button(vita2d_pgf *font, float x, float y, float w, float h, const char *label, int active, int focused);
void gui_draw_circle_outline(float x, float y, float radius, float stroke_width, unsigned int color);
void gui_draw_progress_bar(float x, float y, float w, float h, float progress, unsigned int fill_color, unsigned int bg_color);
void gui_draw_disclaimer(GuiState *state);

#endif
