/* QWERTZ (DE) - PS/2 Set 1 (indices = scancode values) */
#define SCANCODE_A       0x1E
#define SCANCODE_B       0x30
#define SCANCODE_C       0x2E
#define SCANCODE_D       0x20
#define SCANCODE_E       0x12
#define SCANCODE_F       0x21
#define SCANCODE_G       0x22
#define SCANCODE_H       0x23
#define SCANCODE_I       0x17
#define SCANCODE_J       0x24
#define SCANCODE_K       0x25
#define SCANCODE_L       0x26
#define SCANCODE_M       0x32
#define SCANCODE_N       0x31
#define SCANCODE_O       0x18
#define SCANCODE_P       0x19
#define SCANCODE_Q       0x10
#define SCANCODE_R       0x13
#define SCANCODE_S       0x1F
#define SCANCODE_T       0x14
#define SCANCODE_U       0x16
#define SCANCODE_V       0x2F
#define SCANCODE_W       0x11
#define SCANCODE_X       0x2D
#define SCANCODE_Y       0x15
#define SCANCODE_Z       0x2C

// Scancodes für Sonderzeichen
#define SCANCODE_1       0x02
#define SCANCODE_2       0x03
#define SCANCODE_3       0x04
#define SCANCODE_4       0x05
#define SCANCODE_5       0x06
#define SCANCODE_6       0x07
#define SCANCODE_7       0x08
#define SCANCODE_8       0x09
#define SCANCODE_9       0x0A
#define SCANCODE_0       0x0B
#define SCANCODE_MINUS   0x0C
#define SCANCODE_EQUAL   0x0D

// Scancodes für Modifier-Tasten
#define SCANCODE_LEFT_SHIFT  0x2A
#define SCANCODE_RIGHT_SHIFT 0x36
#define SCANCODE_CTRL        0x1D
#define SCANCODE_ALT         0x38

// Scancodes für Sondertasten (z.B. Enter, Escape)
#define SCANCODE_ENTER       0x1C
#define SCANCODE_ESC         0x01
#define SCANCODE_BACKSPACE   0x0E
#define SCANCODE_TAB         0x0F
#define SCANCODE_SPACE       0x39
#define SCANCODE_CAPSLOCK    0x3A

#define SCANCODE_NUMPAD_0    0x52
#define SCANCODE_NUMPAD_1    0x4F
#define SCANCODE_NUMPAD_2    0x50
#define SCANCODE_NUMPAD_3    0x51
#define SCANCODE_NUMPAD_4    0x4B
#define SCANCODE_NUMPAD_5    0x4C
#define SCANCODE_NUMPAD_6    0x4D
#define SCANCODE_NUMPAD_7    0x47
#define SCANCODE_NUMPAD_8    0x48
#define SCANCODE_NUMPAD_9    0x49

static char scancode_ascii[128] = {
    /* 0x00 - 0x0f */
    0, 27, '1','2','3','4','5','6','7','8','9','0','\xDF','\xB4','\b','\t',
    /* 0x10 - 0x1f */
    'q','w','e','r','t','z','u','i','o','p','\xFC','+','\n',0,'a','s',
    /* 0x20 - 0x2f */
    'd','f','g','h','j','k','l','\xF6','\xE4','^',0,'\\','y','x','c','v',
    /* 0x30 - 0x3f */
    'b','n','m',',','.','-',0,'*',0,' ', /* <-- 0x39 = SPACE */ 0,0,0,0,0,0
    /* rest are zero-initialized */
};

static char scancode_ascii_shift[128] = {
    /* 0x00 - 0x0f */
    0, 27, '!','"','\xA7','$','%','&','/','(',')','=','?','`','\b','\t',
    /* 0x10 - 0x1f */
    'Q','W','E','R','T','Z','U','I','O','P','\xDC','*','\n',0,'A','S',
    /* 0x20 - 0x2f */
    'D','F','G','H','J','K','L','\xD6','\xC4','\xB0',0,'|','Y','X','C','V',
    /* 0x30 - 0x3f */
    'B','N','M',';',':','_',0,'*',0,' ', /* <-- 0x39 = SPACE */ 0,0,0,0,0,0
    /* rest are zero-initialized */
};