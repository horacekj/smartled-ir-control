#include <xc.h>
#include "simdelay.h"
#include "smartled.h"

#pragma config WDTEN = OFF
#pragma config FOSC = INTIO7
#pragma config MCLRE = EXTMCLR
#pragma config FCMEN = ON

void __interrupt(high_priority) MyHighIsr(void) {
    if (PIR1bits.TMR1IF) {
        //sl_tmr1_overflow();
        LATDbits.LD1 = !LATDbits.LD1;
        TMR1 = 0xFFFF - 13;
        PIR1bits.TMR1IF = 0;        
    }}

void __interrupt(low_priority) MyLowIsr(void) {

}


void init() {
    OSCCON = (OSCCON & 0b10001111) | 0b01110000;    // internal oscillator at full speed (16 MHz)
    OSCTUNEbits.PLLEN = 1; // PLL 4x, speed = 64 MHz

    TRISD = 0b00000000; // everything out
    TRISAbits.TRISA0 = 0;
    LATD = 0b00000000;
    ANSELB = 0;         // no ADC inputs
    ANSELD = 0;
    
	INTCONbits.GIE = 1;         // enable global interrupts
	INTCONbits.GIEL = 1;        // enable low-priority interrupts
    INTCONbits.GIEH = 1;        // enable high-priority interrupts
    RCONbits.IPEN = 1;          // allow interrupts globally
}

RGB led_red(uint16_t ledi, void* data) {
    return sl_rgb(0x33, 0x00, 0x00);
}

RGB led_rainbow(uint16_t ledi, void* data) {
    return sl_rgb(0x88, 0x00, 0x00);
}

void main(void) {
    init();    
    
    while (true) {
        sl_set_leds(&led_red, NULL);
        DelayMs(16);
    }
}

