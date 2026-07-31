#ifndef TAB_REPORT_H
#define TAB_REPORT_H

#include "gui.h"

void tab_report_init(void);
void tab_report_draw(GuiState *state);
void tab_report_handle_input(GuiState *state, uint32_t pressed_buttons);

#endif
