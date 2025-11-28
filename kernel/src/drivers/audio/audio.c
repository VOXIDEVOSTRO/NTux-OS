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
    beep(523, 180);   // C5
    beep(659, 180);   // E5
    beep(784, 220);   // G5

    beep(1047, 180);  // C6 (glänzt wie XP)
    beep(988, 160);   // B5
    beep(1175, 260);  // D6

    beep(1568, 340);  // G6 - warmes Ende

    beep(0, 100);
}

void play_shutdown_sound(void)
{
    beep(1318, 200);  // E6
    beep(1175, 220);  // D6
    beep(1047, 260);  // C6
    beep( 880, 320);  // A5
    beep( 698, 420);  // F5
    beep( 587, 500);  // D5
    beep( 440, 700);  // A4 - trauriges Ende

    beep(0, 200);
    kprint("Goodbye...\n");
}
void play_slainewin_easteregg(void)
{
    for (uint64_t t = 0; t < 8000 * 60; t++) {  
        uint32_t v = (t * ((t / 2 >> 10 | t % 16 * t >> 8) & 8 * t >> 12 & 18)) | (-(t / 16) + 64);
        uint32_t freq = (v & 0xFF) + 150;  
        if (freq > 150) {
            uint32_t div = 1193180 / freq;  
            outb(0x43, 0xB6); 
            outb(0x42, (uint8_t)div);  
            outb(0x42, div >> 8);  
            outb(0x61, inb(0x61) | 3); 
        }

        for (volatile int d = 0; d < 125; d++) asm volatile ("nop");  
        if ((t % 8000) == 7999) {
            outb(0x61, inb(0x61) & 0xFC);  
            for (volatile int d = 0; d < 3000; d++) asm("nop");  
        }
    }
    outb(0x61, inb(0x61) & 0xFC);
    kprint("SlaineWin Bytebeat sound completed\n");
}