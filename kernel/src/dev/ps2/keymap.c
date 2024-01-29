// clang-format off
// Danish keyboard, some characters are represented with a space, because they take more than 1 byte
// And I am lazy to support them
char kbd_dn[128] =
{
  0,
  ' ', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '+', ' ', '\b',
  '\t', 'q', 'w', 'e', 'r',	't', 'y', 'u', 'i', 'o', 'p', ' ', ' ', '\n',
  0, /* 29 - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ' ',	' ', '\'',
  0, /* Left shift */
  '<', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '-',
  0, /* Right shift */
  0, // No clue what this was supposed to be (NCWTWSTB)
  0,	/* Alt */
  ' ',	/* Space bar */
  0,	/* Caps lock */
  0,	/* 59 - F1 key ... > */
  0,   0,   0,   0,   0,   0,   0,   0,
  0,	/* < ... F10 */
  0,	/* 69 - Num lock*/
  0,	/* Scroll Lock */
  0,	/* Home key */
  0,	/* Up Arrow */
  0,	/* Page Up */
  0, // NCWTWSTB
  0,	/* Left Arrow */
  0,
  0,	/* Right Arrow */
  0, // NCWTWSTB
  0,	/* 79 - End key*/
  0,	/* Down Arrow */
  0,	/* Page Down */
  0,	/* Insert Key */
  0,	/* Delete Key */
  0,   0,   0, // NCWTWSTB
  0,	/* F11 Key */
  0,	/* F12 Key */
  0	/* All other keys are undefined */
};
char kbd_dn_shifted[128] =
{
  0,
  ' ', '!', '"', '#', ' ', '%', '&', '/', '(', ')', '=', '?', '`', '\b',
  '\t',
  'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', ' ', '^', '\r',
  0,			/* 29   - Control */
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ' ', ' ', '*',
  0, /* Left shift */
  '>', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ';', ':', '_',
  0, /* Right shift */
  0,
  0,	/* Alt */
  ' ',	/* Space bar */
  0,	/* Caps lock */
  0,	/* 59 - F1 key ... > */
  0,   0,   0,   0,   0,   0,   0,   0,
  0,	/* < ... F10 */
  0,	/* 69 - Num lock*/
  0,	/* Scroll Lock */
  0,	/* Home key */
  0,	/* Up Arrow */
  0,	/* Page Up */
  0,
  0,	/* Left Arrow */
  0,
  0,	/* Right Arrow */
  0,
  0,	/* 79 - End key*/
  0,	/* Down Arrow */
  0,	/* Page Down */
  0,	/* Insert Key */
  0,	/* Delete Key */
  0,   0,   0,
  0,	/* F11 Key */
  0,	/* F12 Key */
  0	/* All other keys are undefined */
};
char kbd_dn_alt[128] =
{
  0,
  ' ', ' ', '@', ' ', '$', ' ', ' ', '{', '[', ']', '}', ' ', '|', '\b',
  '\t',
  '@', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '"', '~', '\n',
  0, /* 29   - Control */
  ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\'', '^', ' ',
  0, /* Left shift */
  '\\', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
  0, /* Right shift */
  0,
  0,	/* Alt */
  0,	/* Space bar */
  0,	/* Caps lock */
  0,	/* 59 - F1 key ... > */
  0,   0,   0,   0,   0,   0,   0,   0,
  0,	/* < ... F10 */
  0,	/* 69 - Num lock*/
  0,	/* Scroll Lock */
  0,	/* Home key */
  0,	/* Up Arrow */
  0,	/* Page Up */
  0,
  0,	/* Left Arrow */
  0,
  0,	/* Right Arrow */
  0,
  0,	/* 79 - End key*/
  0,	/* Down Arrow */
  0,	/* Page Down */
  0,	/* Insert Key */
  0,	/* Delete Key */
  0,   0,   0,
  0,	/* F11 Key */
  0,	/* F12 Key */
  0	/* All other keys are undefined */
};
// clang-format on