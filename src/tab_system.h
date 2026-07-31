#ifndef TAB_SYSTEM_H
#define TAB_SYSTEM_H

#include "gui.h"

void tab_system_init(void);
void tab_system_finish(void);
void tab_system_draw(GuiState *state);
void tab_system_handle_input(GuiState *state, uint32_t pressed_buttons);

#endif
