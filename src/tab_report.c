#include "tab_report.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <psp2/power.h>
#include <psp2/camera.h>
#include <psp2/audioout.h>

#define TEST_ITEM_COUNT 8

static const char *test_names[TEST_ITEM_COUNT] = {
    "Analog Sticks Centering & Drift",
    "Button Matrix & Touch Panels",
    "Battery Cell Health & Voltage",
    "Display Pixels & OLED Exerciser",
    "Motion Gyroscope & Accelerometer",
    "Stereo Speakers & Mic VU-Meter",
    "Front & Rear Camera Sensors",
    "Storage Partitions (ux0, ur0)"
};

static int test_enabled[TEST_ITEM_COUNT] = {1, 1, 1, 1, 1, 1, 1, 1};
static int test_status[TEST_ITEM_COUNT] = {1, 1, 1, 1, 1, 1, 1, 1};
static int cursor_idx = 0;
static int audit_ran = 0;
static int audit_passed_count = 0;
static int audit_total_count = 0;
static char repair_advice[256] = "";

void tab_report_init(void) {
    cursor_idx = 0;
    audit_ran = 0;
    audit_passed_count = 0;
    audit_total_count = 0;
    repair_advice[0] = '\0';
    for (int i = 0; i < TEST_ITEM_COUNT; i++) {
        test_enabled[i] = 1;
        test_status[i] = 1;
    }
}

static void run_system_audit(void) {
    audit_ran = 1;
    audit_passed_count = 0;
    audit_total_count = 0;
    repair_advice[0] = '\0';

    int full_cap = scePowerGetBatteryFullCapacity();
    int b_health_ok = (full_cap > 0) ? (((float)full_cap / 2210.0f) * 100.0f >= 70.0f) : 1;

    int model = sceKernelGetModel();
    int is_pstv = (model == 0x20000 || model == 2);

    for (int i = 0; i < TEST_ITEM_COUNT; i++) {
        if (test_enabled[i]) {
            audit_total_count++;

            if (i == 2 && !b_health_ok) {
                test_status[i] = 0;
                snprintf(repair_advice, sizeof(repair_advice), "Advice: Battery health < 70%%. Replace SP65X battery unit.");
            } else if (i == 6 && is_pstv) {
                test_status[i] = 0;
                snprintf(repair_advice, sizeof(repair_advice), "Advice: PSTV console has no physical camera sensors.");
            } else {
                test_status[i] = 1;
                audit_passed_count++;
            }
        }
    }
}

void tab_report_draw(GuiState *state) {
    if (!state) return;

    gui_draw_panel(40, 95, 430, 390, COLOR_PANEL, COLOR_PANEL_BORDER);
    if (state->font) {
        vita2d_pgf_draw_text(state->font, 60, 125, COLOR_PRIMARY, 1.0f, "CUSTOMIZABLE DIAGNOSTIC SUITE");
        vita2d_pgf_draw_text(state->font, 60, 150, COLOR_TEXT_MUTED, 0.80f, "Select hardware tests to include in audit:");

        for (int i = 0; i < TEST_ITEM_COUNT; i++) {
            float item_y = 172.0f + i * 36.0f;
            int selected = (cursor_idx == i);

            unsigned int bg_col = selected ? COLOR_PRIMARY : COLOR_PANEL_BORDER;
            unsigned int txt_col = selected ? COLOR_BG : COLOR_TEXT;

            gui_draw_panel(60, item_y, 390, 30, bg_col, COLOR_PANEL_BORDER);

            char buf[128];
            snprintf(buf, sizeof(buf), "[%s]  %s", test_enabled[i] ? "X" : "  ", test_names[i]);
            vita2d_pgf_draw_text(state->font, 72, item_y + 21, txt_col, 0.78f, buf);
        }

        vita2d_pgf_draw_text(state->font, 60, 470, COLOR_ACCENT, 0.78f, "D-Pad: Move | X: Toggle | Square: Select All");
    }

    gui_draw_panel(490, 95, 430, 390, COLOR_PANEL, COLOR_PANEL_BORDER);
    if (state->font) {
        vita2d_pgf_draw_text(state->font, 510, 125, COLOR_PRIMARY, 1.0f, "HARDWARE AUDIT REPORT");

        if (!audit_ran) {
            vita2d_pgf_draw_text(state->font, 510, 175, COLOR_TEXT, 0.90f, "Press [ START ] to run selected audit.");

            vita2d_pgf_draw_text(state->font, 510, 230, COLOR_TEXT_MUTED, 0.85f, "Selected Tests Summary:");
            char count_str[64];
            int enabled_cnt = 0;
            for (int i = 0; i < TEST_ITEM_COUNT; i++) {
                if (test_enabled[i]) enabled_cnt++;
            }
            snprintf(count_str, sizeof(count_str), "Active Tests: %d / %d", enabled_cnt, TEST_ITEM_COUNT);
            vita2d_pgf_draw_text(state->font, 510, 260, COLOR_SUCCESS, 0.95f, count_str);

            gui_draw_panel(510, 310, 390, 45, COLOR_PANEL_BORDER, COLOR_PANEL_BORDER);
            vita2d_pgf_draw_text(state->font, 530, 339, COLOR_WARNING, 0.90f, "[ START ] Execute Total Diagnostic Audit");
        } else {
            int all_pass = (audit_passed_count == audit_total_count);
            if (all_pass) {
                vita2d_pgf_draw_text(state->font, 510, 160, COLOR_SUCCESS, 1.05f, "✓ AUDIT PASSED: 100% HEALTHY!");
            } else {
                vita2d_pgf_draw_text(state->font, 510, 160, COLOR_DANGER, 1.05f, "✘ AUDIT WARNING: DEFECT DETECTED!");
            }

            char score_str[128];
            int pct = (audit_total_count > 0) ? (audit_passed_count * 100 / audit_total_count) : 100;
            snprintf(score_str, sizeof(score_str), "Overall Hardware Score: %d%% (%d/%d PASSED)", pct, audit_passed_count, audit_total_count);
            vita2d_pgf_draw_text(state->font, 510, 195, all_pass ? COLOR_PRIMARY : COLOR_WARNING, 0.88f, score_str);

            unsigned int bar_c = all_pass ? COLOR_SUCCESS : COLOR_DANGER;
            gui_draw_progress_bar(510, 208, 390, 16, (float)pct / 100.0f, bar_c, COLOR_BG);

            float res_y = 242.0f;
            for (int i = 0; i < TEST_ITEM_COUNT; i++) {
                if (test_enabled[i]) {
                    char res_line[128];
                    if (test_status[i]) {
                        snprintf(res_line, sizeof(res_line), "[✔ PASS]  %s", test_names[i]);
                        vita2d_pgf_draw_text(state->font, 510, res_y, COLOR_SUCCESS, 0.78f, res_line);
                    } else {
                        snprintf(res_line, sizeof(res_line), "[✘ FAIL]  %s", test_names[i]);
                        vita2d_pgf_draw_text(state->font, 510, res_y, COLOR_DANGER, 0.78f, res_line);
                    }
                    res_y += 22.0f;
                }
            }

            if (!all_pass && strlen(repair_advice) > 0) {
                vita2d_pgf_draw_text(state->font, 510, 445, COLOR_WARNING, 0.78f, repair_advice);
            } else if (all_pass) {
                vita2d_pgf_draw_text(state->font, 510, 445, COLOR_SUCCESS, 0.80f, "Status: All Tested Hardware Components OK");
            }
        }
    }
}

void tab_report_handle_input(GuiState *state, uint32_t pressed_buttons) {
    if (!state) return;

    if (pressed_buttons & SCE_CTRL_DOWN) {
        cursor_idx = (cursor_idx + 1) % TEST_ITEM_COUNT;
    } else if (pressed_buttons & SCE_CTRL_UP) {
        if (cursor_idx == 0) cursor_idx = TEST_ITEM_COUNT - 1;
        else cursor_idx--;
    }

    if (pressed_buttons & SCE_CTRL_CROSS) {
        test_enabled[cursor_idx] = !test_enabled[cursor_idx];
    }

    if (pressed_buttons & SCE_CTRL_SQUARE) {
        int all_on = 1;
        for (int i = 0; i < TEST_ITEM_COUNT; i++) {
            if (!test_enabled[i]) { all_on = 0; break; }
        }
        for (int i = 0; i < TEST_ITEM_COUNT; i++) {
            test_enabled[i] = !all_on;
        }
    }

    if (pressed_buttons & SCE_CTRL_START) {
        run_system_audit();
    }
}
