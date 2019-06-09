#include <xc.h>
#include "simdelay.h"
#include "smartled.h"
#include "ir.h"

#pragma config WDTEN = OFF
#pragma config FOSC = INTIO7
#pragma config MCLRE = EXTMCLR
#pragma config FCMEN = ON
#pragma config CCP2MX = 0

typedef void (*smartled_mode_t)();
volatile smartled_mode_t smartled_mode;

void mode_off();
void mode_red();
void mode_rgb_moving();

void __interrupt(high_priority) MyHighIsr(void) {

}

void __interrupt(low_priority) MyLowIsr(void) {
    if (PIR1bits.TMR1IF) {
        ir_timer_interrupt();
        PIR1bits.TMR1IF = 0;
    }
    
    if (PIR2bits.CCP2IF) {
        ir_edge_interrupt();
        PIR2bits.CCP2IF = 0;
    }
}

void ir_received(uint8_t addr, uint8_t command) {
    static smartled_mode_t last_mode = &mode_rgb_moving;
    
    LATD |= command;
    
    if (command == 0) { // ON/OFF button        
        if (smartled_mode == mode_off) {
            smartled_mode = last_mode;
        } else {
            last_mode = smartled_mode;
            smartled_mode = &mode_off;
        }
    } else if (command == 6) // 1
        smartled_mode = &mode_red;
    else if (command == 5) // 2
        smartled_mode = &mode_rgb_moving;
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

////////////////////////////////////////////////////////////////////////////////

RGB led_red(uint16_t ledi, void* data) {
    return sl_rgb(0x33, 0x00, 0x00);
}

void mode_red() {
    sl_set_leds(&led_red, NULL);
}

////////////////////////////////////////////////////////////////////////////////

RGB led_rgb_moving(uint16_t ledi, void* data) {
    uint8_t offset = (uint8_t)data;
    RGB rgb;
    rgb.r = (ledi % 3 == ((0+offset) % 3)) ? 0x0A : 0;
    rgb.g = (ledi % 3 == ((1+offset) % 3)) ? 0x0A : 0;
    rgb.b = (ledi % 3 == ((2+offset) % 3)) ? 0x0A : 0;
    return rgb;
}

void mode_rgb_moving() {
    static uint8_t counter = 0;
    
    if (counter == 0)
        sl_set_leds(&led_rgb_moving, (void*)0);
    else if (counter == 2)
        sl_set_leds(&led_rgb_moving, (void*)1);
    else if (counter == 4)
        sl_set_leds(&led_rgb_moving, (void*)2);
    
    counter++;
    
    if (counter == 6)
        counter = 0;
}

////////////////////////////////////////////////////////////////////////////////

RGB led_off(uint16_t ledi, void* data) {
    return sl_rgb(0x00, 0x00, 0x00);
}

void mode_off() {
    sl_set_leds(&led_off, NULL);
}

////////////////////////////////////////////////////////////////////////////////

void main(void) {
    smartled_mode = &mode_off;
    init();
    
    while (true) {
        smartled_mode();
        DelayMs(100);
    }
}

