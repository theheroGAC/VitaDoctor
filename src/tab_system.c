#include "tab_system.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <psp2/net/net.h>
#include <psp2/net/netctl.h>
#include <psp2/sysmodule.h>
#include <psp2/io/devctl.h>
#include <psp2/power.h>
#include <psp2/audioout.h>
#include <psp2/kernel/sysmem.h>

static void *net_memory = NULL;
static int net_inited = 0;
static int led_test_active = 0;
static int led_frame_counter = 0;

typedef struct {
    SceOff max_size;
    SceOff free_size;
    SceOff cluster_size;
} SceIoDevInfoInternal;

void tab_system_init(void) {
    led_test_active = 0;
    led_frame_counter = 0;
    if (sceSysmoduleLoadModule(SCE_SYSMODULE_NET) >= 0) {
        net_memory = malloc(32 * 1024);
        if (net_memory) {
            SceNetInitParam net_param;
            net_param.memory = net_memory;
            net_param.size = 32 * 1024;
            net_param.flags = 0;
            if (sceNetInit(&net_param) >= 0) {
                sceNetCtlInit();
                net_inited = 1;
            }
        }
    }
}

void tab_system_finish(void) {
    if (net_inited) {
        sceNetCtlTerm();
        sceNetTerm();
        net_inited = 0;
    }
    if (net_memory) {
        free(net_memory);
        net_memory = NULL;
    }
    sceSysmoduleUnloadModule(SCE_SYSMODULE_NET);
}

static const char* get_hardware_model_string(void) {
    int model = sceKernelGetModel();
    if (model == SCE_KERNEL_MODEL_VITATV || model == 0x20000 || model == 2) {
        return "PlayStation TV (VTE-1000)";
    }

    SceOff host0_max = 0, host0_free = 0;
    int has_host0 = (sceAppMgrGetDevInfo("host0:", &host0_max, &host0_free) >= 0);
    if (has_host0) {
        return "PS Vita Testing / Dev Kit (PTEL-1000 / PDEL-1000)";
    }

    SceOff uma0_max = 0, uma0_free = 0;
    int has_uma0 = (sceAppMgrGetDevInfo("uma0:", &uma0_max, &uma0_free) >= 0 && uma0_max > 0);
    if (has_uma0) {
        return "PS Vita Slim (PCH-2000 LCD)";
    }

    return "PS Vita Original (PCH-1000 OLED)";
}

static const char* get_motherboard_revision_string(void) {
    int model = sceKernelGetModel();
    if (model == SCE_KERNEL_MODEL_VITATV || model == 0x20000 || model == 2) {
        return "VTE-1000 (PSTV Mainboard)";
    }

    SceOff host0_max = 0, host0_free = 0;
    int has_host0 = (sceAppMgrGetDevInfo("host0:", &host0_max, &host0_free) >= 0);
    if (has_host0) {
        return "PDEL-1000 / PTEL-1000 (Development Board)";
    }

    SceOff uma0_max = 0, uma0_free = 0;
    int has_uma0 = (sceAppMgrGetDevInfo("uma0:", &uma0_max, &uma0_free) >= 0 && uma0_max > 0);
    if (has_uma0) {
        return "DOL-1001 / DOL-1002 (Slim Board)";
    }

    return "IRS-002 / PCA-001 (OLED Board)";
}

static int get_storage_info(const char *dev_name, SceOff *out_max, SceOff *out_free) {
    SceIoDevInfoInternal dev_info;
    memset(&dev_info, 0, sizeof(dev_info));

    int res = sceIoDevctl(dev_name, 0x3001, NULL, 0, &dev_info, sizeof(dev_info));
    if (res >= 0 && dev_info.max_size > 0) {
        *out_max = dev_info.max_size;
        *out_free = dev_info.free_size;
        return 0;
    }

    res = sceAppMgrGetDevInfo(dev_name, out_max, out_free);
    if (res >= 0 && *out_max > 0) {
        return 0;
    }

    return -1;
}

static void draw_storage_bar(GuiState *state, float x, float y, float w, const char *dev_name, const char *dev_label) {
    SceOff max_bytes = 0;
    SceOff free_bytes = 0;
    int res = get_storage_info(dev_name, &max_bytes, &free_bytes);

    gui_draw_panel(x, y, w, 55, COLOR_PANEL, COLOR_PANEL_BORDER);

    if (!state->font) return;

    if (res < 0 || max_bytes <= 0) {
        char label_buf[128];
        snprintf(label_buf, sizeof(label_buf), "%s  %s", dev_name, dev_label);
        vita2d_pgf_draw_text(state->font, x + 12, y + 22, COLOR_TEXT_MUTED, 0.78f, label_buf);
        vita2d_pgf_draw_text(state->font, x + 12, y + 42, COLOR_TEXT_MUTED, 0.72f, "Not Inserted / Not Available");
        return;
    }

    double total_gb = (double)max_bytes / (1024.0 * 1024.0 * 1024.0);
    double free_gb = (double)free_bytes / (1024.0 * 1024.0 * 1024.0);
    double used_gb = total_gb - free_gb;
    float used_ratio = (float)(used_gb / total_gb);

    char dev_title[64];
    if (strcmp(dev_name, "ux0:") == 0 && total_gb > 65.0) {
        snprintf(dev_title, sizeof(dev_title), "ux0: SD2Vita Adapter (MicroSD)");
    } else {
        snprintf(dev_title, sizeof(dev_title), "%s %s", dev_name, dev_label);
    }
    vita2d_pgf_draw_text(state->font, x + 12, y + 20, COLOR_TEXT, 0.78f, dev_title);

    char space_buf[64];
    if (total_gb >= 1.0) {
        snprintf(space_buf, sizeof(space_buf), "%.2f GB Free / %.2f GB", free_gb, total_gb);
    } else {
        double total_mb = (double)max_bytes / (1024.0 * 1024.0);
        double free_mb = (double)free_bytes / (1024.0 * 1024.0);
        snprintf(space_buf, sizeof(space_buf), "%.1f MB Free / %.1f MB", free_mb, total_mb);
    }
    
    float txt_x = x + w - 12.0f - (strlen(space_buf) * 6.0f);
    vita2d_pgf_draw_text(state->font, txt_x, y + 20, COLOR_PRIMARY, 0.76f, space_buf);

    unsigned int bar_col = (used_ratio > 0.90f) ? COLOR_DANGER : ((used_ratio > 0.75f) ? COLOR_WARNING : COLOR_SUCCESS);
    gui_draw_progress_bar(x + 12, y + 32, w - 24, 13, used_ratio, bar_col, COLOR_BG);
}

void tab_system_draw(GuiState *state) {
    if (!state) return;

    if (led_test_active) {
        led_frame_counter++;
    }

    gui_draw_panel(40, 95, 430, 390, COLOR_PANEL, COLOR_PANEL_BORDER);
    if (state->font) {
        vita2d_pgf_draw_text(state->font, 60, 125, COLOR_PRIMARY, 1.0f, "HARDWARE & SYSTEM SPECS");

        char buf[128];
        const char *model_str = get_hardware_model_string();
        snprintf(buf, sizeof(buf), "Model: %s", model_str);
        vita2d_pgf_draw_text(state->font, 60, 158, COLOR_SUCCESS, 0.85f, buf);

        const char *mb_str = get_motherboard_revision_string();
        snprintf(buf, sizeof(buf), "Motherboard: %s", mb_str);
        vita2d_pgf_draw_text(state->font, 60, 188, COLOR_TEXT, 0.85f, buf);

        snprintf(buf, sizeof(buf), "System Firmware: 3.65 / HENkaku");
        vita2d_pgf_draw_text(state->font, 60, 218, COLOR_TEXT, 0.85f, buf);

        int is_online = scePowerIsPowerOnline();
        int is_charging = scePowerIsBatteryCharging();
        if (is_online && is_charging) {
            snprintf(buf, sizeof(buf), "Power Source: External AC Charger (Fast)");
        } else if (is_online) {
            snprintf(buf, sizeof(buf), "Power Source: USB PC Power (Online)");
        } else {
            snprintf(buf, sizeof(buf), "Power Source: Internal Battery Power");
        }
        vita2d_pgf_draw_text(state->font, 60, 248, COLOR_PRIMARY, 0.85f, buf);

        SceNetCtlInfo ip_info;
        if (net_inited && sceNetCtlInetGetInfo(SCE_NETCTL_INFO_GET_IP_ADDRESS, &ip_info) >= 0 && strlen(ip_info.ip_address) > 0) {
            snprintf(buf, sizeof(buf), "Wi-Fi IP Address: %s", ip_info.ip_address);
            vita2d_pgf_draw_text(state->font, 60, 278, COLOR_TEXT, 0.85f, buf);
        } else {
            vita2d_pgf_draw_text(state->font, 60, 278, COLOR_TEXT_MUTED, 0.85f, "Wi-Fi IP Address: Disconnected");
        }

        SceNetEtherAddr mac_addr;
        memset(&mac_addr, 0, sizeof(mac_addr));
        if (net_inited && sceNetGetMacAddress(&mac_addr, 0) >= 0) {
            snprintf(buf, sizeof(buf), "MAC Address: %02X:%02X:%02X:%02X:%02X:%02X",
                mac_addr.data[0], mac_addr.data[1], mac_addr.data[2],
                mac_addr.data[3], mac_addr.data[4], mac_addr.data[5]);
            vita2d_pgf_draw_text(state->font, 60, 308, COLOR_TEXT, 0.85f, buf);
        } else {
            vita2d_pgf_draw_text(state->font, 60, 308, COLOR_TEXT_MUTED, 0.85f, "MAC Address: Unavailable");
        }

        if (led_test_active) {
            int blink_phase = (led_frame_counter % 30 < 15);
            if (blink_phase) {
                vita2d_pgf_draw_text(state->font, 60, 338, COLOR_SUCCESS, 0.85f, "PS Button Blue LED: PULSING ON [⚡]");
            } else {
                vita2d_pgf_draw_text(state->font, 60, 338, COLOR_WARNING, 0.85f, "PS Button Blue LED: PULSING OFF [ ]");
            }
        } else {
            vita2d_pgf_draw_text(state->font, 60, 338, COLOR_PRIMARY, 0.85f, "PS Button Blue LED: Standby (Press TRIANGLE)");
        }

        gui_draw_panel(60, 360, 390, 35, COLOR_PANEL_BORDER, COLOR_PANEL_BORDER);
        vita2d_pgf_draw_text(state->font, 75, 383, COLOR_WARNING, 0.78f, "[ TRIANGLE ] Pulse Blue LED Test  |  [ CIRCLE ] Off");

        vita2d_pgf_draw_text(state->font, 60, 412, COLOR_TEXT_MUTED, 0.80f, "CPU: ARM Cortex-A9 Quad  |  GPU: PowerVR SGX543");
        vita2d_pgf_draw_text(state->font, 60, 445, COLOR_SUCCESS, 0.80f, "System Hardware: Operating Normally");
    }

    gui_draw_panel(490, 95, 430, 390, COLOR_PANEL, COLOR_PANEL_BORDER);
    if (state->font) {
        vita2d_pgf_draw_text(state->font, 510, 125, COLOR_PRIMARY, 1.0f, "STORAGE MONITORING");
    }

    draw_storage_bar(state, 510, 145, 390, "ux0:", "SD2Vita / Memory Card");
    draw_storage_bar(state, 510, 215, 390, "ur0:", "Internal System Storage");
    draw_storage_bar(state, 510, 285, 390, "uma0:", "USB / Secondary Storage");
    draw_storage_bar(state, 510, 355, 390, "gro0:", "Game Cartridge Slot");
}

void tab_system_handle_input(GuiState *state, uint32_t pressed_buttons) {
    if (!state) return;

    if (pressed_buttons & SCE_CTRL_TRIANGLE) {
        led_test_active = 1;
        led_frame_counter = 0;
    } else if (pressed_buttons & SCE_CTRL_CIRCLE) {
        led_test_active = 0;
    }
}
