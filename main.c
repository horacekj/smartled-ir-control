#include <xc.h>
#include "simdelay.h"

#pragma config WDTEN = OFF
#pragma config FOSC = INTIO7
#pragma config MCLRE = EXTMCLR
#pragma config FCMEN = ON

void init() {
    OSCCON = (OSCCON & 0b10001111) | 0b01110000;    // internal oscillator at full speed (16 MHz)

    TRISB = 0b11111111; // five buttons in + unused + PGC, PGD
    LATB = 0xff;        // pull-up by default
    ANSELB = 0;         // no ADC inputs
    
    TRISD = 0b11000000; // LEDs: 0..5 out; TX2: 6 out (but should be set to '1'); RX2: 7 in (UART2 / USB)
    LATD = 0b00000000;
    ANSELD = 0;
    
}

void main(void) {
    init();
    
    for(;;){
        
       LATD = ~LATD;         
           
       DelayMs(200);
       DelayMs(200);
       DelayMs(200);
       DelayMs(200);
       DelayMs(200);
    }
}

