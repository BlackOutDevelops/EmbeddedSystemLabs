// Using Timer_A with 2 channels
// Using ACLK @ 32 KHz (undivided)
// Channel 0 toggles the red LED every 0.1 seconds
// Channel 1 toggles the green LED every 0.5 seconds
#include <msp430fr6989.h>
#define redLED BIT0         // Red at P1.0
#define greenLED BIT7       // Green at P9.7

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;       // Stop WDT
    PM5CTL0 &= ~LOCKLPM5;           // Enable GPIO pins

    P1DIR |= redLED;
    P9DIR |= greenLED;
    P1OUT &= ~redLED;
    P9OUT &= ~greenLED;

    // Configure Channel 0
    TA0CCR0 = 3277-1;               // @ 32 KHz --> 0.1 seconds
    TA0CCTL0 |= CCIE;
    TA0CCTL0 &= ~CCIFG;

    // Configure Channel 1 (write 3 lines similar to above)
    TA1CCR1 = 3277-1;               // @ 32 KHz --> 0.1 seconds
    TA1CCTL1 |= CCIE;
    TA1CCTL1 &= ~CCIFG;

    // Configure timer (ACLK) (divide by 1) (continuous mode)
    TA0CTL = TASSEL_1 | ID_0 | MC_2 | TACLR;

    // Engage a low-power mode...
    return;
}

// ISR of Channel 0 (A0 vector)
#pragma vector = TIMER0_A0_VECTOR
__interrupt void T0A0_ISR() {
    P1OUT ^= redLED;                // Toggle the red LED
    TA0CCR0 += 3277;                // Schedule the next interrupt

    // Hardware clears Channel 0 flag (CCIFG in TA0CCTL0)
}

// ISR of Channel 1 (A1 vector) ... fill the vector name below
#pragma vector = TIMER1_A1_VECTOR
__interrupt void T0A1_ISR() {
    P9OUT ^= greenLED;                         // Toggle the green LED
    TA1CCR1 += 3277;                         // Schedule the next interrupt
    TA1CTL = ~TAIFG;                         // Clear Channel 1 interrupt flag
}
