#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>

void beep(uint32_t frequency_hz, uint32_t duration_ms);
void play_startup_sound(void);
void play_shutdown_sound(void);
void play_slainewin_easteregg(void);

#endif