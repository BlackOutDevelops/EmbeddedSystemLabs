// Turning on the red LED while button S1 is pushed
#include <msp430fr6989.h>
#define redLED BIT0             // Red LED at P1.0
#define greenLED BIT7           // Green LED at P9.7
#define BUT1 BIT1               // Button S1 at P1.112

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;     // Stop the Watchdog timer
    PM5CTL0 &= ~LOCKLPM5;         // Enable the GPIO pins

    // Configure and initialize LEDs

    P1DIR |= redLED;              // Direct pin as output
    P9DIR |= greenLED;            // Direct pin as output
    P1OUT &= ~redLED;             // Turn LED Off
    P9OUT &= ~greenLED;           // Turn LED Off

    // Configure buttons

    P1DIR &= ~BUT1;               // Direct pin as input
    P1REN |= BUT1;                // Enable built-in resistor
    P1OUT |= BUT1;                // Set resistor as pull-up

    // Polling the button in an infinite loop

    for(;;) {

        // Fill the if-statement below...

        if ((P1IN & BUT1) == 0)
            P1OUT |= redLED;     // Turn red LED on
        else
            P1OUT &= ~redLED;   // Turn red LED off
    }
}
