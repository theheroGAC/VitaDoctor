#include "tab_audio.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

static int audio_port = -1;
static int mic_port = -1;
static int active_test = 0;
static float mic_volume_level = 0.0f;

#define AUDIO_SAMPLES 1024
#define MIC_GRAIN 512

static int16_t sample_buf[AUDIO_SAMPLES * 2];
static int16_t mic_buf[MIC_GRAIN];

static int16_t *left_pcm_buf = NULL;
static size_t left_pcm_samples = 0;
static size_t left_pcm_index = 0;

static int16_t *right_pcm_buf = NULL;
static size_t right_pcm_samples = 0;
static size_t right_pcm_index = 0;

static int load_wav_file(const char *filepath, int16_t **out_pcm, size_t *out_samples) {
    FILE *f = fopen(filepath, "rb");
    if (!f) return -1;

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (fsize <= 44) {
        fclose(f);
        return -1;
    }

    size_t pcm_bytes = fsize - 44;
    int16_t *pcm = (int16_t *)malloc(pcm_bytes);
    if (!pcm) {
        fclose(f);
        return -1;
    }

    fseek(f, 44, SEEK_SET);
    size_t read_bytes = fread(pcm, 1, pcm_bytes, f);
    fclose(f);

    *out_pcm = pcm;
    *out_samples = read_bytes / 4;
    return 0;
}

static void load_tones(void) {
    const char *left_paths[] = {
        "app0:audio/sound_left.wav",
        "app0:/audio/sound_left.wav",
        "app0:sound_left.wav",
        "ux0:app/VITADOC00/audio/sound_left.wav",
        NULL
    };

    for (int i = 0; left_paths[i] != NULL; i++) {
        if (load_wav_file(left_paths[i], &left_pcm_buf, &left_pcm_samples) == 0) {
            break;
        }
    }

    const char *right_paths[] = {
        "app0:audio/sound_right.wav",
        "app0:/audio/sound_right.wav",
        "app0:sound_right.wav",
        "ux0:app/VITADOC00/audio/sound_right.wav",
        NULL
    };

    for (int i = 0; right_paths[i] != NULL; i++) {
        if (load_wav_file(right_paths[i], &right_pcm_buf, &right_pcm_samples) == 0) {
            break;
        }
    }
}

void tab_audio_init(void) {
    audio_port = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_MAIN, AUDIO_SAMPLES, 44100, SCE_AUDIO_OUT_PARAM_FORMAT_S16_STEREO);
    if (audio_port < 0) {
        audio_port = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_BGM, AUDIO_SAMPLES, 44100, SCE_AUDIO_OUT_PARAM_FORMAT_S16_STEREO);
    }

    if (audio_port >= 0) {
        int vol[2] = {32768, 32768};
        sceAudioOutSetVolume(audio_port, (SCE_AUDIO_VOLUME_FLAG_L_CH | SCE_AUDIO_VOLUME_FLAG_R_CH), vol);
    }

    mic_port = sceAudioInOpenPort(SCE_AUDIO_IN_PORT_TYPE_VOICE, MIC_GRAIN, 16000, SCE_AUDIO_IN_PARAM_FORMAT_S16_MONO);
    if (mic_port < 0) {
        mic_port = sceAudioInOpenPort(SCE_AUDIO_IN_PORT_TYPE_RAW, MIC_GRAIN, 16000, SCE_AUDIO_IN_PARAM_FORMAT_S16_MONO);
    }

    active_test = 0;
    mic_volume_level = 0.0f;
    left_pcm_index = 0;
    right_pcm_index = 0;

    load_tones();
}

void tab_audio_finish(void) {
    if (audio_port >= 0) {
        sceAudioOutReleasePort(audio_port);
        audio_port = -1;
    }
    if (mic_port >= 0) {
        sceAudioInReleasePort(mic_port);
        mic_port = -1;
    }

    if (left_pcm_buf) {
        free(left_pcm_buf);
        left_pcm_buf = NULL;
    }
    if (right_pcm_buf) {
        free(right_pcm_buf);
        right_pcm_buf = NULL;
    }
}

static void update_audio_and_mic(void) {
    if (audio_port >= 0 && active_test > 0) {
        int vol[2] = {32768, 32768};
        sceAudioOutSetVolume(audio_port, (SCE_AUDIO_VOLUME_FLAG_L_CH | SCE_AUDIO_VOLUME_FLAG_R_CH), vol);

        if (active_test == 1 && left_pcm_buf && left_pcm_samples > 0) {
            for (int i = 0; i < AUDIO_SAMPLES; i++) {
                sample_buf[i * 2] = left_pcm_buf[left_pcm_index * 2];
                sample_buf[i * 2 + 1] = left_pcm_buf[left_pcm_index * 2 + 1];

                left_pcm_index++;
                if (left_pcm_index >= left_pcm_samples) left_pcm_index = 0;
            }
            sceAudioOutOutput(audio_port, sample_buf);
        } else if (active_test == 2 && right_pcm_buf && right_pcm_samples > 0) {
            for (int i = 0; i < AUDIO_SAMPLES; i++) {
                sample_buf[i * 2] = right_pcm_buf[right_pcm_index * 2];
                sample_buf[i * 2 + 1] = right_pcm_buf[right_pcm_index * 2 + 1];

                right_pcm_index++;
                if (right_pcm_index >= right_pcm_samples) right_pcm_index = 0;
            }
            sceAudioOutOutput(audio_port, sample_buf);
        } else {
            static float phase = 0.0f;
            float freq = (active_test == 1) ? 440.0f : 880.0f;
            float phase_step = (2.0f * 3.14159265f * freq) / 44100.0f;

            for (int i = 0; i < AUDIO_SAMPLES; i++) {
                int16_t val = (int16_t)(sinf(phase) * 30000.0f);
                phase += phase_step;
                if (phase >= 2.0f * 3.14159265f) phase -= 2.0f * 3.14159265f;

                if (active_test == 1) {
                    sample_buf[i * 2] = val;
                    sample_buf[i * 2 + 1] = 0;
                } else {
                    sample_buf[i * 2] = 0;
                    sample_buf[i * 2 + 1] = val;
                }
            }
            sceAudioOutOutput(audio_port, sample_buf);
        }
    }

    if (mic_port >= 0) {
        int ret = sceAudioInInput(mic_port, mic_buf);
        if (ret >= 0) {
            float sum = 0.0f;
            for (int i = 0; i < MIC_GRAIN; i++) {
                float val = fabsf((float)mic_buf[i]);
                if (val > sum) sum = val;
            }
            float target_level = sum / 32768.0f;
            mic_volume_level = (mic_volume_level * 0.7f) + (target_level * 0.3f);
        }
    }
}

void tab_audio_draw(GuiState *state) {
    if (!state) return;

    update_audio_and_mic();

    gui_draw_panel(40, 100, 420, 380, COLOR_PANEL, COLOR_PANEL_BORDER);
    if (state->font) {
        vita2d_pgf_draw_text(state->font, 60, 130, COLOR_PRIMARY, 1.0f, "STEREO SPEAKER TEST");

        vita2d_pgf_draw_text(state->font, 60, 165, COLOR_TEXT, 0.85f, "Test Left & Right Speakers:");

        unsigned int l_bg = (active_test == 1) ? COLOR_SUCCESS : COLOR_PANEL_BORDER;
        gui_draw_panel(60, 185, 380, 45, l_bg, COLOR_PANEL_BORDER);
        vita2d_pgf_draw_text(state->font, 80, 214, COLOR_TEXT, 0.85f, "[ SQUARE ] Play Left Speaker Tone (440Hz)");

        unsigned int r_bg = (active_test == 2) ? COLOR_SUCCESS : COLOR_PANEL_BORDER;
        gui_draw_panel(60, 240, 380, 45, r_bg, COLOR_PANEL_BORDER);
        vita2d_pgf_draw_text(state->font, 80, 269, COLOR_TEXT, 0.85f, "[ CIRCLE ] Play Right Speaker Tone (880Hz)");

        gui_draw_panel(60, 295, 380, 40, COLOR_PANEL_BORDER, COLOR_PANEL_BORDER);
        vita2d_pgf_draw_text(state->font, 80, 322, COLOR_WARNING, 0.85f, "[ CROSS ] Stop Audio Tone");

        char status_str[128];
        snprintf(status_str, sizeof(status_str), "Audio Port Status: %s", (audio_port >= 0) ? "ACTIVE (OK)" : "ERROR");
        vita2d_pgf_draw_text(state->font, 60, 365, (audio_port >= 0) ? COLOR_SUCCESS : COLOR_DANGER, 0.8f, status_str);

        snprintf(status_str, sizeof(status_str), "Left WAV Tone: %s", (left_pcm_buf && left_pcm_samples > 0) ? "LOADED" : "SINE FALLBACK");
        vita2d_pgf_draw_text(state->font, 60, 390, COLOR_TEXT_MUTED, 0.78f, status_str);

        snprintf(status_str, sizeof(status_str), "Right WAV Tone: %s", (right_pcm_buf && right_pcm_samples > 0) ? "LOADED" : "SINE FALLBACK");
        vita2d_pgf_draw_text(state->font, 60, 412, COLOR_TEXT_MUTED, 0.78f, status_str);

        if (active_test == 1) {
            vita2d_pgf_draw_text(state->font, 60, 455, COLOR_SUCCESS, 0.85f, "Playing tone on LEFT speaker...");
        } else if (active_test == 2) {
            vita2d_pgf_draw_text(state->font, 60, 455, COLOR_SUCCESS, 0.85f, "Playing tone on RIGHT speaker...");
        } else {
            vita2d_pgf_draw_text(state->font, 60, 455, COLOR_TEXT_MUTED, 0.85f, "Audio test idle. Press Square/Circle.");
        }
    }

    gui_draw_panel(490, 100, 430, 380, COLOR_PANEL, COLOR_PANEL_BORDER);
    if (state->font) {
        vita2d_pgf_draw_text(state->font, 510, 130, COLOR_PRIMARY, 1.0f, "MICROPHONE VU-METER");

        vita2d_pgf_draw_text(state->font, 510, 170, COLOR_TEXT, 0.9f, "Speak into the PS Vita microphone:");

        unsigned int bar_col = (mic_volume_level > 0.75f) ? COLOR_DANGER :
                              ((mic_volume_level > 0.40f) ? COLOR_WARNING : COLOR_SUCCESS);
        
        gui_draw_progress_bar(510, 210, 390, 30, mic_volume_level, bar_col, COLOR_BG);

        char buf[64];
        snprintf(buf, sizeof(buf), "Input Level: %d%%", (int)(mic_volume_level * 100.0f));
        vita2d_pgf_draw_text(state->font, 510, 275, COLOR_TEXT, 0.9f, buf);

        if (mic_volume_level > 0.03f) {
            vita2d_pgf_draw_text(state->font, 510, 320, COLOR_SUCCESS, 0.9f, "✓ Microphone Signal Detected!");
        } else {
            vita2d_pgf_draw_text(state->font, 510, 320, COLOR_TEXT_MUTED, 0.85f, "Waiting for sound input...");
        }

        snprintf(buf, sizeof(buf), "Microphone Hardware: %s", (mic_port >= 0) ? "CONNECTED (OK)" : "ERROR");
        vita2d_pgf_draw_text(state->font, 510, 430, (mic_port >= 0) ? COLOR_SUCCESS : COLOR_DANGER, 0.8f, buf);
    }
}

void tab_audio_handle_input(GuiState *state, uint32_t pressed_buttons) {
    if (!state) return;

    if (pressed_buttons & SCE_CTRL_SQUARE) {
        active_test = (active_test == 1) ? 0 : 1;
        left_pcm_index = 0;
    } else if (pressed_buttons & SCE_CTRL_CIRCLE) {
        active_test = (active_test == 2) ? 0 : 2;
        right_pcm_index = 0;
    } else if (pressed_buttons & SCE_CTRL_CROSS) {
        active_test = 0;
    }
}
