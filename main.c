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
volatile uint8_t color_intensity = 0x33;
volatile RGB color_same;

void mode_red();
void mode_green();
void mode_yellow();
void mode_blue();
void mode_white();
void mode_rgb_moving();
void mode_off();
void mode_gradient();

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
    } else if (command == 75) // red button
        smartled_mode = &mode_red;
    else if (command == 76) // green button
        smartled_mode = &mode_green;
    else if (command == 77) // yellow button
        smartled_mode = &mode_yellow;
    else if (command == 78) // blue button
        smartled_mode = &mode_blue;
    else if (command == 5) // 1
        smartled_mode = &mode_rgb_moving;
    else if (command == 6) // 2
        smartled_mode = &mode_gradient;
    else if (command == 7) // 3
        smartled_mode = &mode_white;
    else if (command == 16) { // arrow down
        if (color_intensity >= 10)
            color_intensity -= 10;
    } else if (command == 12) { // arrow up
        if (color_intensity <= 245)
            color_intensity += 10;
    }
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

RGB led_same(uint16_t ledi, void* data) {
    return (*(RGB*)data);
}

void mode_off(ir_set_func func) { color_same = sl_rgb(0x00, 0x00, 0x00); sl_set_leds(&led_same, (void*)&color_same); }
void mode_red() { color_same = sl_rgb(color_intensity, 0x00, 0x00); sl_set_leds(&led_same, (void*)&color_same); }
void mode_green() { color_same = sl_rgb(0x00, color_intensity, 0x00); sl_set_leds(&led_same, (void*)&color_same); }
void mode_yellow() { color_same = sl_rgb(color_intensity, color_intensity, 0x00); sl_set_leds(&led_same, (void*)&color_same); }
void mode_blue() { color_same = sl_rgb(0x00, 0x00, color_intensity); sl_set_leds(&led_same, (void*)&color_same); }
void mode_white() { color_same = sl_rgb(color_intensity, color_intensity, color_intensity); sl_set_leds(&led_same, (void*)&color_same); }

////////////////////////////////////////////////////////////////////////////////

RGB led_rgb_moving(uint16_t ledi, void* data) {
    uint8_t offset = (uint8_t)data;
    RGB rgb;
    rgb.r = (ledi % 3 == ((0+offset) % 3)) ? color_intensity : 0;
    rgb.g = (ledi % 3 == ((1+offset) % 3)) ? color_intensity : 0;
    rgb.b = (ledi % 3 == ((2+offset) % 3)) ? color_intensity : 0;
    return rgb;
}

void mode_rgb_moving() {
    static uint8_t counter = 0;
    
    if (counter == 0)
        sl_set_leds(&led_rgb_moving, (void*)0);
    else if (counter == 20)
        sl_set_leds(&led_rgb_moving, (void*)1);
    else if (counter == 40)
        sl_set_leds(&led_rgb_moving, (void*)2);
    
    counter++;
    
    if (counter == 60)
        counter = 0;
}

////////////////////////////////////////////////////////////////////////////////

void mode_gradient() {
    static uint8_t counter = 0;
    
    if (counter <= 85) {
        color_same = sl_rgb(
                color_intensity - color_intensity*(float)counter/85,
                color_intensity*(float)counter/85,
                0x00
        );
    } else if (counter > 170) {
        color_same = sl_rgb(                
                color_intensity*(float)(counter-170)/85,
                0x00,
                color_intensity - color_intensity*(float)(counter-170)/85
        );
    } else {
        color_same = sl_rgb(
                0x00,
                color_intensity - color_intensity*(float)(counter-85)/85,                
                color_intensity*(float)(counter-85)/85
        );        
    }
    
    sl_set_leds(&led_same, (void*)&color_same);
    
    counter += 2;
    if (counter >= 250)
        counter = 0;
}

////////////////////////////////////////////////////////////////////////////////

void main(void) {
    smartled_mode = &mode_off;
    init();
    
    while (true) {
        smartled_mode();
        DelayMs(10);
    }
}

