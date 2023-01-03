// Timer_A continuous mode, with interrupt, flashes LEDs
#include <msp430fr6989.h>
#define redLED BIT0             // Red LED at P1.0
#define greenLED BIT7           // Green LED at P9.7

void T0A1_ISR();
void config_ACLK_to_32KHz_crystal();

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;     // Stop the Watchdog timer
    PM5CTL0 &= ~LOCKLPM5;         // Enable the GPIO pins
    P1DIR |= redLED;              // Direct pin as output
    P9DIR |= greenLED;            // Direct pin as output
    P1OUT &= ~redLED;             // Turn LED Off
    P9OUT &= ~greenLED;           // Turn LED Off

    // Configure ACLK to the 32 KHz crystal

    config_ACLK_to_32KHz_crystal();

    // Timer_A configuration (fill the line below)
    // Use ACLK, divide by 1, continuous mode, TAR cleared, enableinterrupt for rollback-to-zero event

    TA0CTL = TASSEL_1 | ID_0 | MC_2 | TACLR | TAIE;

    // Ensure the flag is cleared at the start

    TA0CTL &= ~TAIFG;

    // Enable the global interrupt bit (call an intrinsic function)

    _enable_interrupts();

    // Infinite loop... the code waits here between interrupts

    for(;;) {}

//    // Low Power Mode
//
//    _low_power_mode_3();
}

//*******Writing the ISR*******
#pragma vector = TIMER0_A1_VECTOR     // Link the ISR to the vector
__interrupt void T0A1_ISR() {
    // Toggle both LEDs

    P1OUT ^= redLED;
    P9OUT ^= greenLED;

    // Clear the TAIFG flag

    TA0CTL &= ~TAIFG;
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

