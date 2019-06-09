# SmartLED control via IR remote

This project was created as a final project of PB171 course @ FI MUNI, spring
2019.

Main aim: connect IR receiver & smart LEDs. Allow to set multiple modes of smart
leds via IR remote controller.

## Basic info

 * CPU: PIC18F46K22
 * Board: YuniPIC v1.1
 * IDE: MPLABX
 * Compiler: XC8
 * Remote controller protocol: [NEC IR](https://exploreembedded.com/wiki/NEC_IR_Remote_Control_Interface_with_8051)
 * Smart LEDs: 46 pcs of WS2812B

## Wiring

 * LEDs' data to RA0 (ADC0 on YuniPIC, disconnect ADC jumper).
 * Remote control receiver to RB3 (BUTT4 on YuniPIC, unsolder BUTT4's capacitor)
