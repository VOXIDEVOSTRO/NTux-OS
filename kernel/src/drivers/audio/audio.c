#include <drivers/audio/audio.h>
#include <kernel_lib/io.h>

static uint8_t speaker_was_on = 0;

void beep(uint32_t freq_hz, uint32_t duration_ms)
{
    if (freq_hz == 0) {
        // Turn speaker off
        outb(0x61, inb(0x61) & 0xFC);
        speaker_was_on = 0;
        return;
    }

    // 1. Force-enable the speaker bit FIRST (very important in QEMU!)
    uint8_t tmp = inb(0x61);
    if (!(tmp & 0x03)) {
        outb(0x61, tmp | 0x03);   // bit 0 = speaker data, bit 1 = enable PIT2 output
        speaker_was_on = 1;
    }

    // 2. Program PIT Channel 2 (square wave)
    uint32_t divisor = 1193180 / freq_hz;
    outb(0x43, 0xB6);               // Channel 2, lo/hi, mode 3 (square wave), binary
    outb(0x42, divisor & 0xFF);
    outb(0x42, divisor >> 8);

    // 3. Simple calibrated delay (works on all QEMU versions)
    // ~1 ms per 1 000 000 iterations on typical QEMU speed
    for (volatile uint64_t i = 0; i < duration_ms * 1000000ULL; i++);

    // 4. Turn off (only if we turned it on ourselves)
    if (speaker_was_on) {
        outb(0x61, inb(0x61) & 0xFC);
        speaker_was_on = 0;
    }
}

void play_startup_sound(void)
{
    beep( 523, 140);   // C5
    beep( 659, 140);   // E5
    beep( 880, 140);   // A5
    beep(1047, 140);   // C6
    beep(1318, 140);   // E6
    beep(1760, 140);   // A6

    beep(2093, 180);   // C7
    beep(2349, 180);   // D7
    beep(2794, 700);   // F7 

    beep(0, 100);
}

void play_shutdown_sound(void)
{
    beep(1760, 200);   // A6
    beep(1480, 200);   // F6
    beep(1175, 250);   // D6
    beep( 988, 300);   // B5
    beep( 784, 300);   // G5
    beep( 659, 400);   // E5
    beep( 523, 900);   // C5
    kprint("Goodbye!\n");
}