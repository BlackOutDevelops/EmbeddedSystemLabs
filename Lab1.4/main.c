// Code that flashes the red LED
#include <msp430fr6989.h>
#define redLED BIT0             // Red LED at P1.0
#define greenLED BIT7             // Green LED at P9.7


void main(void) {
    volatile unsigned int i;
    WDTCTL = WDTPW | WDTHOLD;     // Stop the Watchdog timer
    PM5CTL0 &= ~LOCKLPM5;         // Disable GPIO power-on default high-impedance mode

    P1DIR |= redLED;              // Direct pin as output
    P1OUT &= ~redLED;             // Turn redLED Off

    P9DIR |= greenLED;            // Direct pin as output
    P9OUT |= greenLED;           // Turn greenLED on
//    P9OUT &= ~greenLED;           // Turn greenLED off

    for(;;) {
        // Delay loop
        for(i=0; i<20000; i++) {}
        P1OUT ^= redLED;            // Toggle the redLED
        P9OUT ^= greenLED;          // Toggle the greenLED
    }
}
