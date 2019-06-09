#include <stddef.h>
#include "smartled.h"

set_func sl_current_set_func = NULL;
void* sl_current_data = NULL;

RGB sl_rgb(uint8_t r, uint8_t g, uint8_t b) {
    RGB colors = { .r = r, .g = g, .b = b };
    return colors;
}

void sl_init() {
    
}

void sl_set_leds(set_func f, void* data) {
    sl_current_set_func = f;
    sl_current_data = data;
}

