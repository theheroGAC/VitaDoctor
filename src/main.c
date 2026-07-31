#include <vitasdk.h>
#include <vita2d.h>
#include <stdio.h>
#include <stdlib.h>

#include "gui.h"
#include "tab_analog.h"
#include "tab_battery.h"
#include "tab_display.h"
#include "tab_motion.h"
#include "tab_audio.h"
#include "tab_camera.h"
#include "tab_system.h"
#include "tab_report.h"

int main(int argc, char *argv[]) {
    vita2d_init();
    vita2d_set_clear_color(COLOR_BG);

    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG_WIDE);
    sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);
    sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK, SCE_TOUCH_SAMPLING_STATE_START);

    GuiState state;
    gui_init(&state);

    tab_battery_init();
    tab_display_init();
    tab_motion_init();
    tab_audio_init();
    tab_camera_init();
    tab_system_init();
    tab_report_init();

    SceCtrlData pad;
    SceTouchData touch_front, touch_back;
    uint32_t old_buttons = 0;

    while (1) {
        sceCtrlPeekBufferPositive(0, &pad, 1);
        sceTouchPeek(SCE_TOUCH_PORT_FRONT, &touch_front, 1);
        sceTouchPeek(SCE_TOUCH_PORT_BACK, &touch_back, 1);

        uint32_t pressed_buttons = pad.buttons & ~old_buttons;
        old_buttons = pad.buttons;

        if ((pad.buttons & SCE_CTRL_SELECT) && (pad.buttons & SCE_CTRL_START)) {
            break;
        }

        if (state.show_disclaimer) {
            if (pressed_buttons || pad.buttons != 0 || touch_front.reportNum > 0) {
                state.show_disclaimer = 0;
            }
        } else {
            if (state.current_tab == TAB_ANALOG) {
                if ((pad.buttons & SCE_CTRL_SELECT) && (pressed_buttons & (SCE_CTRL_LTRIGGER | SCE_CTRL_L1))) {
                    state.current_tab = TAB_COUNT - 1;
                } else if ((pad.buttons & SCE_CTRL_SELECT) && (pressed_buttons & (SCE_CTRL_RTRIGGER | SCE_CTRL_R1))) {
                    state.current_tab = (state.current_tab + 1) % TAB_COUNT;
                }
            } else {
                if (pressed_buttons & (SCE_CTRL_LTRIGGER | SCE_CTRL_L1)) {
                    if (state.current_tab == 0) state.current_tab = TAB_COUNT - 1;
                    else state.current_tab--;
                } else if (pressed_buttons & (SCE_CTRL_RTRIGGER | SCE_CTRL_R1)) {
                    state.current_tab = (state.current_tab + 1) % TAB_COUNT;
                }
            }

            if (state.current_tab == TAB_ANALOG) {
                tab_analog_handle_input(&state, &pad, pressed_buttons);
            } else if (state.current_tab == TAB_DISPLAY) {
                tab_display_handle_input(&state, pressed_buttons);
            } else if (state.current_tab == TAB_AUDIO) {
                tab_audio_handle_input(&state, pressed_buttons);
            } else if (state.current_tab == TAB_CAMERA) {
                tab_camera_handle_input(&state, pressed_buttons);
            } else if (state.current_tab == TAB_SYSTEM) {
                tab_system_handle_input(&state, pressed_buttons);
            } else if (state.current_tab == TAB_REPORT) {
                tab_report_handle_input(&state, pressed_buttons);
            }
        }

        vita2d_start_drawing();
        vita2d_clear_screen();

        gui_draw_header(&state, "Hardware Diagnostic");
        gui_draw_tabs(&state);

        switch (state.current_tab) {
            case TAB_ANALOG:
                tab_analog_draw(&state, &pad, &touch_front, &touch_back);
                break;
            case TAB_BATTERY:
                tab_battery_draw(&state);
                break;
            case TAB_DISPLAY:
                tab_display_draw(&state, &pad);
                break;
            case TAB_MOTION:
                tab_motion_draw(&state);
                break;
            case TAB_AUDIO:
                tab_audio_draw(&state);
                break;
            case TAB_CAMERA:
                tab_camera_draw(&state);
                break;
            case TAB_SYSTEM:
                tab_system_draw(&state);
                break;
            case TAB_REPORT:
                tab_report_draw(&state);
                break;
            default:
                break;
        }

        gui_draw_disclaimer(&state);

        vita2d_end_drawing();
        vita2d_swap_buffers();
    }

    vita2d_wait_rendering_done();
    gui_finish(&state);
    tab_audio_finish();
    tab_camera_finish();
    tab_system_finish();
    sceMotionStopSampling();
    vita2d_fini();
    sceKernelExitProcess(0);
    return 0;
}
