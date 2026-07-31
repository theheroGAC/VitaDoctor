#ifndef TAB_CAMERA_H
#define TAB_CAMERA_H

#include "gui.h"

void tab_camera_init(void);
void tab_camera_finish(void);
void tab_camera_draw(GuiState *state);
void tab_camera_handle_input(GuiState *state, uint32_t pressed_buttons);

#endif
