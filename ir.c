#include <xc.h>
#include <pic18.h>
#include "ir.h"

void ir_init() {
    // Initialize capture on pin RB3
    TRISBbits.TRISB3 = 1; // IR receiver on RB3
    CCP2CONbits.CCP2M = 0b0100; // capture on falling edge
    CCPTMRS0bits.C2TSEL = 0b00; // capure uses timer1
    IPR2bits.CCP2IP = 1; // interrupt high priority
    PIE2bits.CCP2IE = 1; // enable capture interrupt    
    
    // Timer 1 setup, increment at 2 MHz
    T1CONbits.TMR1CS = 0b00;    // clock source instruction clock (Fosc/4)
    T1CONbits.T1CKPS = 0b11;    // prescaler 8x (f=500kHz)
    PIE1bits.TMR1IE = 1;        // enable interrupt
    IPR1bits.TMR1IP = 1;        // interrupt high priority
    T1CONbits.TMR1ON = 1;       // enable timer1        
}

void ir_timer_interrupt() {
    
}

void ir_edge_interrupt() {
    LATDbits.LD0 = !LATDbits.LD0;
}