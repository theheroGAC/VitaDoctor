#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void config_set_default(VitaDoctorConfig *cfg) {
    if (!cfg) return;
    cfg->left_inner_deadzone = 10;
    cfg->left_outer_deadzone = 5;
    cfg->right_inner_deadzone = 10;
    cfg->right_outer_deadzone = 5;
    cfg->deadzone_shape = 0; // Radial
    cfg->enable_plugin = 1;
}

int config_load(VitaDoctorConfig *cfg) {
    if (!cfg) return 0;
    config_set_default(cfg);

    FILE *f = fopen(CONFIG_PATH, "r");
    if (!f) {
        // Try fallback directory
        f = fopen("ux0:data/VitaDoctor/config.ini", "r");
        if (!f) return 0;
    }

    char line[128];
    while (fgets(line, sizeof(line), f)) {
        char key[64];
        int val;
        if (sscanf(line, "%63[^=]=%d", key, &val) == 2) {
            if (strcmp(key, "left_inner_deadzone") == 0) cfg->left_inner_deadzone = val;
            else if (strcmp(key, "left_outer_deadzone") == 0) cfg->left_outer_deadzone = val;
            else if (strcmp(key, "right_inner_deadzone") == 0) cfg->right_inner_deadzone = val;
            else if (strcmp(key, "right_outer_deadzone") == 0) cfg->right_outer_deadzone = val;
            else if (strcmp(key, "deadzone_shape") == 0) cfg->deadzone_shape = val;
            else if (strcmp(key, "enable_plugin") == 0) cfg->enable_plugin = val;
        }
    }
    fclose(f);
    return 1;
}

int config_save(const VitaDoctorConfig *cfg) {
    if (!cfg) return 0;

    sceIoMkdir(CONFIG_DIR, 0777);
    sceIoMkdir("ur0:tai", 0777);

    FILE *f = fopen(CONFIG_PATH, "w");
    if (!f) {
        f = fopen("ux0:data/VitaDoctor/config.ini", "w");
        if (!f) return 0;
    }

    fprintf(f, "# VitaDoctor Configuration File\n");
    fprintf(f, "left_inner_deadzone=%d\n", cfg->left_inner_deadzone);
    fprintf(f, "left_outer_deadzone=%d\n", cfg->left_outer_deadzone);
    fprintf(f, "right_inner_deadzone=%d\n", cfg->right_inner_deadzone);
    fprintf(f, "right_outer_deadzone=%d\n", cfg->right_outer_deadzone);
    fprintf(f, "deadzone_shape=%d\n", cfg->deadzone_shape);
    fprintf(f, "enable_plugin=%d\n", cfg->enable_plugin);

    fclose(f);
    return 1;
}
