#ifndef CONFIG_H
#define CONFIG_H

#include <vitasdk.h>

#define CONFIG_PATH "ur0:tai/vitadoctor.ini"
#define CONFIG_DIR  "ux0:data/VitaDoctor"

typedef struct {
    int left_inner_deadzone;   // 0 - 50 (percentage of max stick radius)
    int left_outer_deadzone;   // 0 - 30 (percentage of outer radius threshold)
    int right_inner_deadzone;  // 0 - 50
    int right_outer_deadzone;  // 0 - 30
    int deadzone_shape;        // 0: Radial (circle), 1: Axial (cross)
    int enable_plugin;         // 0: disabled, 1: enabled
} VitaDoctorConfig;

void config_set_default(VitaDoctorConfig *cfg);
int config_load(VitaDoctorConfig *cfg);
int config_save(const VitaDoctorConfig *cfg);

#endif // CONFIG_H
