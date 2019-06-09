#ifndef SMARTLED_H
#define	SMARTLED_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t r, g, b;
} RGB;

#define SL_LEDS_COUNT 46

typedef RGB (*ir_set_func)(uint16_t ledi, void* data);

RGB sl_rgb(uint8_t r, uint8_t g, uint8_t b);
void sl_set_leds(ir_set_func f, void* data);

#endif	/* SMARTLED_H */

