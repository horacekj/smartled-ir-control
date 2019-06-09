#include <stddef.h>
#include <xc.h>
#include "smartled.h"

set_func sl_current_set_func = NULL;
void* sl_current_data = NULL;
uint16_t sl_led_count = 0;
int32_t current_led_index = -1;
uint8_t current_halfbit = 0;

uint8_t reverse_bits(uint8_t b);

uint8_t reverse_bits(uint8_t b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

RGB sl_rgb(uint8_t r, uint8_t g, uint8_t b) {
    RGB colors = { .r = r, .g = g, .b = b };
    return colors;
}

void sl_set_leds(set_func f, void* data) {
    sl_current_set_func = f;
    sl_current_data = data;
    RGB output[SL_LEDS_COUNT];
    for (uint16_t i = 0; i < SL_LEDS_COUNT; i++) {
        output[i] = f(i, data);
        output[i].r = reverse_bits(output[i].r);
        output[i].g = reverse_bits(output[i].g);
        output[i].b = reverse_bits(output[i].b);
    }
    
#define SEND_SINGLE_BIT() if (bits & 1) { \
        LATAbits.LA0 = 1; \
        bits >>= 1; \
        Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); \
        LATAbits.LA0 = 0; \
        Nop(); \
    } else { \
        LATAbits.LA0 = 1; \
        bits >>= 1; \
        Nop(); \
        LATAbits.LA0 = 0; \
        Nop(); Nop(); Nop(); Nop(); Nop(); \
    }
    
    for (uint16_t i = 0; i < SL_LEDS_COUNT; i++) {
        uint24_t bits = ((uint24_t)output[i].b << 16) + ((uint24_t)output[i].r << 8) + (output[i].g);
        
        SEND_SINGLE_BIT();
        SEND_SINGLE_BIT();
        SEND_SINGLE_BIT();
        SEND_SINGLE_BIT();
        SEND_SINGLE_BIT();
        SEND_SINGLE_BIT();
        SEND_SINGLE_BIT();
        SEND_SINGLE_BIT();

        SEND_SINGLE_BIT();
        SEND_SINGLE_BIT();
        SEND_SINGLE_BIT();
        SEND_SINGLE_BIT();
        SEND_SINGLE_BIT();
        SEND_SINGLE_BIT();
        SEND_SINGLE_BIT();
        SEND_SINGLE_BIT();

        SEND_SINGLE_BIT();
        SEND_SINGLE_BIT();
        SEND_SINGLE_BIT();
        SEND_SINGLE_BIT();
        SEND_SINGLE_BIT();
        SEND_SINGLE_BIT();
        SEND_SINGLE_BIT();
        SEND_SINGLE_BIT();
    }
}