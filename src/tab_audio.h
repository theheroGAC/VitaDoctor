#ifndef TAB_AUDIO_H
#define TAB_AUDIO_H

#include "gui.h"

void tab_audio_init(void);
void tab_audio_finish(void);
void tab_audio_draw(GuiState *state);
void tab_audio_handle_input(GuiState *state, uint32_t pressed_buttons);

#endif
