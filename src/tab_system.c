#include "tab_system.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <psp2/net/net.h>
#include <psp2/net/netctl.h>
#include <psp2/sysmodule.h>
#include <psp2/io/devctl.h>

static void *net_memory = NULL;
static int net_inited = 0;

typedef struct {
    SceOff max_size;
    SceOff free_size;
    SceOff cluster_size;
} SceIoDevInfoInternal;

void tab_system_init(void) {
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
    if (model == 0x20000 || model == 2) {
        return "PlayStation TV (VTE-1000)";
    } else if (model == 1) {
        return "PS Vita Slim (PCH-2000 LCD)";
    } else if (model == 0) {
        return "PS Vita Original (PCH-1000 OLED)";
    }
    return "PS Vita Handheld Console";
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
    snprintf(dev_title, sizeof(dev_title), "%s %s", dev_name, dev_label);
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

    gui_draw_panel(40, 95, 430, 390, COLOR_PANEL, COLOR_PANEL_BORDER);
    if (state->font) {
        vita2d_pgf_draw_text(state->font, 60, 125, COLOR_PRIMARY, 1.0f, "HARDWARE & SYSTEM SPECS");

        char buf[128];
        vita2d_pgf_draw_text(state->font, 60, 160, COLOR_TEXT_MUTED, 0.82f, "Console Model:");

        const char *model_str = get_hardware_model_string();
        vita2d_pgf_draw_text(state->font, 60, 185, COLOR_SUCCESS, 0.95f, model_str);

        snprintf(buf, sizeof(buf), "System Firmware: 3.65 / HENkaku");
        vita2d_pgf_draw_text(state->font, 60, 225, COLOR_TEXT, 0.90f, buf);

        SceNetCtlInfo ip_info;
        if (net_inited && sceNetCtlInetGetInfo(SCE_NETCTL_INFO_GET_IP_ADDRESS, &ip_info) >= 0 && strlen(ip_info.ip_address) > 0) {
            snprintf(buf, sizeof(buf), "Wi-Fi IP Address: %s", ip_info.ip_address);
            vita2d_pgf_draw_text(state->font, 60, 265, COLOR_SUCCESS, 0.88f, buf);
        } else {
            vita2d_pgf_draw_text(state->font, 60, 265, COLOR_TEXT_MUTED, 0.88f, "Wi-Fi IP Address: Disconnected");
        }

        SceNetEtherAddr mac_addr;
        memset(&mac_addr, 0, sizeof(mac_addr));
        if (net_inited && sceNetGetMacAddress(&mac_addr, 0) >= 0) {
            snprintf(buf, sizeof(buf), "MAC Address: %02X:%02X:%02X:%02X:%02X:%02X",
                mac_addr.data[0], mac_addr.data[1], mac_addr.data[2],
                mac_addr.data[3], mac_addr.data[4], mac_addr.data[5]);
            vita2d_pgf_draw_text(state->font, 60, 305, COLOR_TEXT, 0.85f, buf);
        } else {
            SceNetCtlInfo mac_info;
            if (net_inited && sceNetCtlInetGetInfo(SCE_NETCTL_INFO_GET_ETHER_ADDR, &mac_info) >= 0) {
                snprintf(buf, sizeof(buf), "MAC Address: %02X:%02X:%02X:%02X:%02X:%02X",
                    mac_info.ether_addr.data[0], mac_info.ether_addr.data[1],
                    mac_info.ether_addr.data[2], mac_info.ether_addr.data[3],
                    mac_info.ether_addr.data[4], mac_info.ether_addr.data[5]);
                vita2d_pgf_draw_text(state->font, 60, 305, COLOR_TEXT, 0.85f, buf);
            } else {
                vita2d_pgf_draw_text(state->font, 60, 305, COLOR_TEXT_MUTED, 0.85f, "MAC Address: Unavailable");
            }
        }

        vita2d_pgf_draw_text(state->font, 60, 355, COLOR_TEXT_MUTED, 0.80f, "Architecture: ARM Cortex-A9 Quad-Core");
        vita2d_pgf_draw_text(state->font, 60, 385, COLOR_TEXT_MUTED, 0.80f, "Graphics GPU: PowerVR SGX543MP4+");
        vita2d_pgf_draw_text(state->font, 60, 415, COLOR_TEXT_MUTED, 0.80f, "Main Memory RAM: 512 MB  |  VRAM: 128 MB");
        vita2d_pgf_draw_text(state->font, 60, 445, COLOR_SUCCESS, 0.80f, "System Status: Operating Normally");
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
