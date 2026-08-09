#include <stdint.h>
#include <zos_time.h>
#include <zvb_sound.h>

#include "feedback.h"

#define FEEDBACK_FREQUENCY 880
#define FEEDBACK_DURATION_MS 40

void feedback_beep(void) {
    uint8_t previous_peripheral = zvb_config_dev_idx;

    zvb_sound_initialize(1);
    zvb_sound_set_voices(
        VOICE3,
        SOUND_FREQ_TO_DIV(FEEDBACK_FREQUENCY),
        WAV_SQUARE | DUTY_CYCLE_50_0
    );
    zvb_sound_set_channels(VOICE3, VOICE3);
    zvb_sound_set_voices_vol(VOICE3, VOL_25);
    zvb_sound_set_volume(VOL_25);
    (void)msleep(FEEDBACK_DURATION_MS);
    zvb_sound_set_hold(VOICE3, 1);
    zvb_sound_set_volume(VOL_0);
    zvb_map_peripheral(previous_peripheral);
}
