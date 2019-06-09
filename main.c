#include <xc.h>
#include "simdelay.h"
#include "smartled.h"
#include "ir.h"

#pragma config WDTEN = OFF
#pragma config FOSC = INTIO7
#pragma config MCLRE = EXTMCLR
#pragma config FCMEN = ON
#pragma config CCP2MX = 0

void __interrupt(high_priority) MyHighIsr(void) {
    if (PIR1bits.TMR1IF) {
        ir_timer_interrupt();
        PIR1bits.TMR1IF = 0;
    }
    
    if (PIR2bits.CCP2IF) {
        ir_edge_interrupt();
        PIR2bits.CCP2IF = 0;
    }
}

void __interrupt(low_priority) MyLowIsr(void) {

}

void ir_received(uint8_t addr, uint8_t command) {
    LATDbits.LD4 = !LATDbits.LD4;
}

void init() {
    OSCCON = (OSCCON & 0b10001111) | 0b01110000;    // internal oscillator at full speed (16 MHz)
    OSCTUNEbits.PLLEN = 1; // PLL 4x, speed = 64 MHz

    TRISD = 0b00000000; // everything out
    TRISAbits.TRISA0 = 0; // LEDS on RA0 (pot1 disconnected)    
    LATD = 0b00000000;
    ANSELB = 0;         // no ADC inputs
    ANSELD = 0;
    
    ir_init(&ir_received);
    
	INTCONbits.GIE = 1;         // enable global interrupts
	INTCONbits.GIEL = 1;        // enable low-priority interrupts
    INTCONbits.GIEH = 1;        // enable high-priority interrupts
    RCONbits.IPEN = 1;          // allow interrupts globally
}

RGB led_red(uint16_t ledi, void* data) {
    return sl_rgb(0x33, 0x00, 0x00);
}

RGB led_mix(uint16_t ledi, void* data) {
    uint8_t offset = (uint8_t)data;
    RGB rgb;
    rgb.r = (ledi % 3 == ((0+offset) % 3)) ? 0x0A : 0;
    rgb.g = (ledi % 3 == ((1+offset) % 3)) ? 0x0A : 0;
    rgb.b = (ledi % 3 == ((2+offset) % 3)) ? 0x0A : 0;
    return rgb;
}

RGB led_data(uint16_t ledi, void* data) {
    
}

void main(void) {
    init();    
    
    while (true) {        
        sl_set_leds(&led_mix, (void*)0);
        DelayMs(250);
        sl_set_leds(&led_mix, (void*)1);
        DelayMs(250);
        sl_set_leds(&led_mix, (void*)2);
        DelayMs(250);
    }
}

