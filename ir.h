#ifndef IR_H
#define	IR_H

#include <stdint.h>

typedef void (*ir_callback_t)(uint8_t addr, uint8_t command);

void ir_init(ir_callback_t callback);
void ir_timer_interrupt();
void ir_edge_interrupt();

#endif	/* IR_H */

