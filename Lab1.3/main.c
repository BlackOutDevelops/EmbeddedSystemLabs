// Code that flashes the red LED
#include <msp430fr6989.h>
#include <stdint.h>
#define redLED BIT0             // Red LED at P1.0

void main(void) {
//    volatile unsigned int i, j; // Variables for nested delay loop
    volatile uint32_t i;    // unsigned int 32-bit type
    WDTCTL = WDTPW | WDTHOLD;     // Stop the Watchdog timer
    PM5CTL0 &= ~LOCKLPM5;         // Disable GPIO power-on default high-impedance mode

    P1DIR |= redLED;              // Direct pin as output
    P1OUT &= ~redLED;             // Turn LED Off

    for(;;) {
        // Nested delay loop
//        for(i=0; i<40000; i++) {
//            for(j=0; j<3; j++) {}}
//        P1OUT ^= redLED;            // Toggle the LED

        // Delay loop
        for(i=0; i<120000; i++) {}
        P1OUT ^= redLED;            // Toggle the LED
    }
}
