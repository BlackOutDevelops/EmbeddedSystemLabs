// CUSTOMIZE YOUR OWN - Continuous Mode
#include <msp430fr6989.h>
#define redLED BIT0             // Red LED at P1.0
#define greenLED BIT7           // Green LED at P9.7

void config_ACLK_to_32KHz_crystal();

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;     // Stop the Watchdog timer
    PM5CTL0 &= ~LOCKLPM5;         // Enable the GPIO pins
    P1DIR |= redLED;              // Direct pin as output
    P9DIR |= greenLED;            // Direct pin as output
    P1OUT &= ~redLED;             // Turn LED Off
    P9OUT |= greenLED;           // Turn LED Off

    // Configure ACLK to the 32 KHz crystal (function call)

    config_ACLK_to_32KHz_crystal();

    // Configure Timer_A
    // Timer_A: ACLK, div by 1, continuous mode, clear TAR

    TA0CTL = TASSEL_1 | ID_0 | MC_2 | TACLR;

    // Ensure flag is cleared at the start

    TA0CTL &= ~TAIFG;

    // Set the initial start of the count for TAR

    TA0R |= 0x8000;

    // Infinite loop

    for(;;) {

        // Empty while loop; waits here until TAIFG is raised

        while(!(TA0CTL & TAIFG)) {}
        P9OUT ^= greenLED;      // Toggle the green LED
        P1OUT ^= redLED;        // Toggle the red LED
        TA0CTL &= ~TAIFG;       // Clear the flag
        TA0R |= 0x8000;         // Reset the TAR to custom value
    }
}

//**********************************
// Configures ACLK to 32 KHz crystal

void config_ACLK_to_32KHz_crystal() {
    // By default, ACLK runs on LFMODCLK at 5MHz/128 = 39 KHz

    // Reroute pins to LFXIN/LFXOUT functionality

    PJSEL1 &= ~BIT4;PJSEL0 |= BIT4;

    // Wait until the oscillator fault flags remain cleared

    CSCTL0 = CSKEY;          // Unlock CS registers

    do {
        CSCTL5 &= ~LFXTOFFG;   // Local fault flag
        SFRIFG1 &= ~OFIFG;     // Global fault flag
    } while((CSCTL5 & LFXTOFFG) != 0);

    CSCTL0_H = 0;            // Lock CS registers
    return;
}
