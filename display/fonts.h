#ifndef FONTS_H
#define FONTS_H

#include <inttypes.h>

typedef enum {
    FONT_HEIGHT = 0,
    FONT_LTSPPOS,
    FONT_CCNT,
    FONT_OFTA,
    FONT_OFTNA,

    FONT_HEADER_END
} FontHeader;

typedef struct {
    const uint8_t *data;

    uint16_t color;

    uint8_t height;

    uint8_t fixed;
    uint8_t mult;
    uint8_t direction;
} Font;

extern const uint8_t font_ks0066_ru_08[] __attribute__((section (".bonus")));
extern const uint8_t font_ks0066_ru_24[] __attribute__((section (".bonus")));
extern const uint8_t font_digits_32[] __attribute__((section (".bonus")));

#endif // FONTS_H
