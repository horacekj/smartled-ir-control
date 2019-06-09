#include <xc.h>
#include <pic18.h>
#include <stdbool.h>
#include "ir.h"
#include "simdelay.h"

typedef enum {
    idle = 0,
    addr = 1,
    addr_inv = 2,
    command = 3,
    command_inv = 4,
} receive_state_t;

typedef struct {
    uint8_t addr;
    uint8_t addr_inv;
    uint8_t command;
    uint8_t command_inv;
} received_data_t;

typedef struct {
    bool valid;
    bool data;
} bit_parse_result;

volatile receive_state_t receive_state;
volatile received_data_t received_data;
volatile uint8_t bit_i;
volatile uint16_t last_tick_time;
volatile bool last_tick_valid = false;
volatile bool going_to_reset = false;
ir_callback_t ir_callback = NULL;

void process_edge(uint16_t period);
bit_parse_result get_bit(uint16_t period);
void check_and_call();

void ir_init(ir_callback_t callback) {
    ir_callback = callback;

    // Initialize capture on pin RB3
    TRISBbits.TRISB3 = 1;       // IR receiver on RB3
    CCP2CONbits.CCP2M = 0b0100; // capture on falling edge
    CCPTMRS0bits.C2TSEL = 0b00; // capture uses timer1
    IPR2bits.CCP2IP = 0;        // interrupt low priority
    PIE2bits.CCP2IE = 1;        // enable capture interrupt

    // Timer 1 setup, increment at 2 MHz, ~ 33 ms to overflow
    T1CONbits.TMR1CS = 0b00;    // clock source instruction clock (Fosc/4)
    T1CONbits.T1CKPS = 0b11;    // prescaler 8x (f=2MHz)
    PIE1bits.TMR1IE = 1;        // enable interrupt
    IPR1bits.TMR1IP = 0;        // interrupt low priority
    T1CONbits.TMR1ON = 1;       // enable timer1
}

void ir_timer_interrupt() {
    // reset when no edge on two successive overflows
    if (going_to_reset) {
        // reset
        last_tick_valid = false;
        receive_state = idle;
        LATDbits.LD7 = 1;
        going_to_reset = false;
        return;
    }
    if (receive_state != idle)
        going_to_reset = true;
}

void ir_edge_interrupt() {
    going_to_reset = false;

    if (!last_tick_valid) {
        last_tick_valid = true;
        last_tick_time = CCPR2;
        return;
    }

    uint16_t time = CCPR2;
    uint16_t period = time - last_tick_time;
    process_edge(period);
    last_tick_time = time;
}

void process_edge(uint16_t period) {
    if ((period > 24000) && (period < 29000)) {
        // period between 12 ms and 14.5 ms (should be 13.5 ms)
        // -> initialize
        receive_state = addr;
        bit_i = 0;
        received_data.addr = 0;
        received_data.addr_inv = 0;
        received_data.command = 0;
        received_data.command_inv = 0;
        LATD = 0;

        return;
    }

    bit_parse_result r = get_bit(period);
    if (!r.valid) {
        LATDbits.LD7 = 1;
        receive_state = idle;
        last_tick_valid = false;
        return;
    }

    if (receive_state == addr)
        received_data.addr |= (r.data << bit_i);
    else if (receive_state == addr_inv)
        received_data.addr_inv |= (r.data << bit_i);
    else if (receive_state == command)
        received_data.command |= (r.data << bit_i);
    else if (receive_state == command_inv)
        received_data.command_inv |= (r.data << bit_i);

    bit_i++;

    if (bit_i == 8) {
        bit_i = 0;
        if (receive_state < command_inv) {
            receive_state++;
        } else {
            last_tick_valid = false;
            receive_state = idle;
            check_and_call();
        }
    }
}

bit_parse_result get_bit(uint16_t period) {
    bit_parse_result r;
    r.valid = true;
    if ((period >= 2000) && (period <= 3000)) // 1 ms to 1.5 ms (should be 1.25)
        r.data = 0;
    else if ((period >= 4000) && (period <= 5000)) // 2 ms to 2.5 ms (should be 2.25)
        r.data = 1;
    else
        r.valid = false;

    return r;
}

void check_and_call() {
    if (received_data.addr != (uint8_t)~(received_data.addr_inv)) {
        LATDbits.LD7 = 1; // DEBUG
        return;
    }
    if (received_data.command != (uint8_t)~received_data.command_inv) {
        LATDbits.LD7 = 1; // DEBUG
        return;
    }

    if (ir_callback != NULL)
        ir_callback(received_data.addr, received_data.command);
}
