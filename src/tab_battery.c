#include "tab_battery.h"
#include <stdio.h>
#include <math.h>

#define DESIGN_CAPACITY_MAH 2210.0f
#define HISTORY_SIZE 60

static float volt_history[HISTORY_SIZE];
static int history_idx = 0;
static uint64_t last_update_time = 0;

void tab_battery_init(void) {
    for (int i = 0; i < HISTORY_SIZE; i++) {
        volt_history[i] = 3700.0f;
    }
    history_idx = 0;
    last_update_time = sceKernelGetProcessTimeWide();
}

void tab_battery_update(void) {
    uint64_t now = sceKernelGetProcessTimeWide();
    if (now - last_update_time >= 1000000) {
        last_update_time = now;
        int volt = scePowerGetBatteryVolt();
        if (volt > 0) {
            volt_history[history_idx] = (float)volt;
            history_idx = (history_idx + 1) % HISTORY_SIZE;
        }
    }
}

void tab_battery_draw(GuiState *state) {
    if (!state) return;

    tab_battery_update();

    int pct = scePowerGetBatteryLifePercent();
    int full_cap = scePowerGetBatteryFullCapacity();
    int volt = scePowerGetBatteryVolt();
    int temp = scePowerGetBatteryTemp();
    int is_charging = scePowerIsBatteryCharging();
    int life_time = scePowerGetBatteryLifeTime();

    float temp_c = temp / 10.0f;
    float temp_f = (temp_c * 9.0f / 5.0f) + 32.0f;

    float health_pct = 100.0f;
    if (full_cap > 0) {
        health_pct = ((float)full_cap / DESIGN_CAPACITY_MAH) * 100.0f;
        if (health_pct > 100.0f) health_pct = 100.0f;
    }

    gui_draw_panel(40, 100, 420, 220, COLOR_PANEL, COLOR_PANEL_BORDER);
    if (state->font) {
        vita2d_pgf_draw_text(state->font, 60, 130, COLOR_PRIMARY, 1.0f, "BATTERY HEALTH & CAPACITY");

        char buf[64];
        snprintf(buf, sizeof(buf), "%d%%", pct);
        unsigned int status_color = (pct > 50) ? COLOR_SUCCESS : ((pct > 20) ? COLOR_WARNING : COLOR_DANGER);
        vita2d_pgf_draw_text(state->font, 60, 185, status_color, 2.2f, buf);

        if (is_charging) {
            vita2d_pgf_draw_text(state->font, 200, 175, COLOR_WARNING, 1.0f, "[ CHARGING ]");
        } else {
            vita2d_pgf_draw_text(state->font, 200, 175, COLOR_TEXT_MUTED, 1.0f, "[ DISCHARGING ]");
        }

        snprintf(buf, sizeof(buf), "Estimated Cell Health: %.1f%%", health_pct);
        vita2d_pgf_draw_text(state->font, 60, 225, COLOR_TEXT, 0.9f, buf);

        gui_draw_progress_bar(60, 240, 380, 18, health_pct / 100.0f, COLOR_SUCCESS, COLOR_BG);

        if (life_time > 0) {
            int hours = life_time / 60;
            int mins = life_time % 60;
            snprintf(buf, sizeof(buf), "Remaining Time: %dh %02dm", hours, mins);
        } else {
            snprintf(buf, sizeof(buf), "Remaining Time: Calculating...");
        }
        vita2d_pgf_draw_text(state->font, 60, 285, COLOR_TEXT_MUTED, 0.85f, buf);
    }

    gui_draw_panel(490, 100, 430, 220, COLOR_PANEL, COLOR_PANEL_BORDER);
    if (state->font) {
        vita2d_pgf_draw_text(state->font, 510, 130, COLOR_PRIMARY, 1.0f, "POWER METRICS");

        char buf[64];
        snprintf(buf, sizeof(buf), "Voltage: %d mV  (%.2f V)", volt, volt / 1000.0f);
        vita2d_pgf_draw_text(state->font, 510, 170, COLOR_TEXT, 0.9f, buf);

        snprintf(buf, sizeof(buf), "Temperature: %.1f deg C / %.1f deg F", temp_c, temp_f);
        unsigned int temp_color = (temp_c > 42.0f) ? COLOR_DANGER : COLOR_TEXT;
        vita2d_pgf_draw_text(state->font, 510, 205, temp_color, 0.9f, buf);

        snprintf(buf, sizeof(buf), "Full Capacity: %d mAh", full_cap > 0 ? full_cap : 2210);
        vita2d_pgf_draw_text(state->font, 510, 240, COLOR_TEXT, 0.9f, buf);

        snprintf(buf, sizeof(buf), "Design Capacity: %d mAh", (int)DESIGN_CAPACITY_MAH);
        vita2d_pgf_draw_text(state->font, 510, 275, COLOR_TEXT_MUTED, 0.9f, buf);
    }

    gui_draw_panel(40, 340, 880, 160, COLOR_PANEL, COLOR_PANEL_BORDER);
    if (state->font) {
        vita2d_pgf_draw_text(state->font, 60, 365, COLOR_PRIMARY, 0.9f, "VOLTAGE HISTORY (LAST 60 SECONDS)");
    }

    float chart_x = 60.0f;
    float chart_y = 380.0f;
    float chart_w = 840.0f;
    float chart_h = 100.0f;

    vita2d_draw_rectangle(chart_x, chart_y, chart_w, chart_h, COLOR_BG);
    gui_draw_panel(chart_x, chart_y, chart_w, chart_h, 0, COLOR_PANEL_BORDER);

    float step_x = chart_w / (HISTORY_SIZE - 1);
    float min_v = 3400.0f;
    float max_v = 4200.0f;

    for (int i = 0; i < HISTORY_SIZE - 1; i++) {
        int idx1 = (history_idx + i) % HISTORY_SIZE;
        int idx2 = (history_idx + i + 1) % HISTORY_SIZE;

        float v1 = volt_history[idx1];
        float v2 = volt_history[idx2];

        float y1 = chart_y + chart_h - ((v1 - min_v) / (max_v - min_v)) * chart_h;
        float y2 = chart_y + chart_h - ((v2 - min_v) / (max_v - min_v)) * chart_h;

        if (y1 < chart_y) y1 = chart_y;
        if (y1 > chart_y + chart_h) y1 = chart_y + chart_h;
        if (y2 < chart_y) y2 = chart_y;
        if (y2 > chart_y + chart_h) y2 = chart_y + chart_h;

        float x1 = chart_x + i * step_x;
        float x2 = chart_x + (i + 1) * step_x;

        vita2d_draw_line(x1, y1, x2, y2, COLOR_PRIMARY);
    }
}
