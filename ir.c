#include <xc.h>
#include <pic18.h>
#include <stdbool.h>
#include "ir.h"

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

volatile receive_state_t receive_state;
volatile received_data_t received_data;
volatile uint8_t bit_i;
volatile uint16_t last_tick_time;
volatile bool last_tick_valid = false;
volatile bool going_to_reset = false;

void process_edge(uint16_t period);

void ir_init() {
    // Initialize capture on pin RB3
    TRISBbits.TRISB3 = 1;       // IR receiver on RB3
    CCP2CONbits.CCP2M = 0b0100; // capture on falling edge
    CCPTMRS0bits.C2TSEL = 0b00; // capture uses timer1
    IPR2bits.CCP2IP = 1;        // interrupt high priority
    PIE2bits.CCP2IE = 1;        // enable capture interrupt    
    
    // Timer 1 setup, increment at 2 MHz, ~ 33 ms to overflow
    T1CONbits.TMR1CS = 0b00;    // clock source instruction clock (Fosc/4)
    T1CONbits.T1CKPS = 0b11;    // prescaler 8x (f=2MHz)
    PIE1bits.TMR1IE = 1;        // enable interrupt
    IPR1bits.TMR1IP = 1;        // interrupt high priority
    T1CONbits.TMR1ON = 1;       // enable timer1        
}

void ir_timer_interrupt() {
    // reset when no edge on two successive overflows
    if (going_to_reset) {
        // reset
        last_tick_valid = false;
        receive_state = idle;
        LATDbits.LD1 = !LATDbits.LD1;
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
    
    uint16_t period = CCPR2 - last_tick_time;
    process_edge(period);
    last_tick_time = CCPR2;
}

void process_edge(uint16_t period) {
    LATDbits.LD0 = !LATDbits.LD0;
    if ((period > 24000) && (period < 29000)) {
        // period between 12 ms and 14.5 ms (should be 13.5 ms)
        // -> initialize
        receive_state = addr;        
        return;
    }
}