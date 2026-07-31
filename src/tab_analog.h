#ifndef TAB_ANALOG_H
#define TAB_ANALOG_H

#include "gui.h"

void tab_analog_draw(GuiState *state, const SceCtrlData *pad, const SceTouchData *touch_front, const SceTouchData *touch_back);
void tab_analog_handle_input(GuiState *state, const SceCtrlData *pad, uint32_t pressed_buttons);

#endif // TAB_ANALOG_H
