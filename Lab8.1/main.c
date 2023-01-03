// Single digit counter and key input through UART
#include <msp430fr6989.h>
#include <stdlib.h>
#define redLED BIT0             // Red LED at P1.0
#define greenLED BIT7           // Green LED at P9.7
#define FLAGS      UCA1IFG      // Contains the transmit & receive flags
#define RXFLAG     UCRXIFG         // Receive flag
#define TXFLAG     UCTXIFG         // Transmit flag
#define TXBUFFER   UCA1TXBUF       // Transmit buffer
#define RXBUFFER   UCA1RXBUF       // Receive buffer
void Initialize_UART(void);
void uart_write_char(unsigned char ch);
unsigned char uart_read_char(void);

void main(void) {
    volatile unsigned int i, key;
    unsigned char ch;
    WDTCTL = WDTPW | WDTHOLD;     // Stop the Watchdog timer
    PM5CTL0 &= ~LOCKLPM5;         // Enable the GPIO pins
    P1DIR |= redLED;              // Direct pin as output
    P9DIR |= greenLED;            // Direct pin as output
    P1OUT &= ~redLED;             // Turn LED Off
    P9OUT &= ~greenLED;           // Turn LED Off

    Initialize_UART();

    for(;;)
    {
        key = uart_read_char();
        for(ch='0';ch<='9';ch++)
        {
            if (key == '1')
                P9OUT |= greenLED;
            else if (key == '2')
                P9OUT &= ~greenLED;

            for(i = 0; i < 2000; i++) {}
            uart_write_char(ch);
            uart_write_char('\n');
            uart_write_char('\r');
            for(i = 0; i < 2000; i++) {}
        }

        P1OUT ^= redLED;
    }
}

// Configure UART to the popular configuration
// 9600 baud, 8-bit data, LSB first, no parity bits, 1 stop bit
// no flow control
// Initial clock: SMCLK @ 1.048 MHz with oversampling
void Initialize_UART(void){
    // Divert pins to UART functionality
    P3SEL1 &= ~(BIT4|BIT5);
    P3SEL0 |= (BIT4|BIT5);

    // Use SMCLK clock; leave other settings default
    UCA1CTLW0 |= UCSSEL_2;

    // Configure the clock dividers and modulators
    // UCBR=6, UCBRF=13, UCBRS=0x22, UCOS16=1 (oversampling)
    UCA1BRW = 6;
    UCA1MCTLW = UCBRS5|UCBRF3|UCOS16;

    // Exit the reset state (so transmission/reception can begin)
    UCA1CTLW0 &= ~UCSWRST;
}

void uart_write_char(unsigned char ch){
    // Wait for any ongoing transmission to complete
    while ( (FLAGS & TXFLAG)==0 ) {}

    // Write the byte to the transmit buffer
    TXBUFFER = ch;
}

// The function returns the byte; if none received, returns NULL
unsigned char uart_read_char(void){
    unsigned char temp;

    // Return NULL if no byte received
    if( (FLAGS & RXFLAG) == 0)
        return NULL;

    // Otherwise, copy the received byte (clears the flag) and return it
    temp = RXBUFFER;

    return temp;
}
