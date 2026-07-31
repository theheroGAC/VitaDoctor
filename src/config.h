#ifndef CONFIG_H
#define CONFIG_H

#include <vitasdk.h>

#define CONFIG_PATH "ur0:tai/vitadoctor.ini"
#define CONFIG_DIR  "ux0:data/VitaDoctor"

typedef struct {
    int left_inner_deadzone;
    int left_outer_deadzone;
    int right_inner_deadzone;
    int right_outer_deadzone;
    int deadzone_shape;
    int enable_plugin;
} VitaDoctorConfig;

void config_set_default(VitaDoctorConfig *cfg);
int config_load(VitaDoctorConfig *cfg);
int config_save(const VitaDoctorConfig *cfg);

#endif
