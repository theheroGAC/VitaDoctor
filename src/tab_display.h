#ifndef TAB_DISPLAY_H
#define TAB_DISPLAY_H

#include "gui.h"

void tab_display_init(void);
void tab_display_draw(GuiState *state, const SceCtrlData *pad);
void tab_display_handle_input(GuiState *state, uint32_t pressed_buttons);

#endif
