// unsigned 16-bit integer counter through UART with 4800 baud rate
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
void Initialize_UART2(void);
void uart_write_uint16(unsigned int n);
void uart_write_char(unsigned char ch);
unsigned char uart_read_char(void);
void config_ACLK_to_32KHz_crystal();

void main(void) {
    volatile unsigned int i, counter = 0;
    WDTCTL = WDTPW | WDTHOLD;     // Stop the Watchdog timer
    PM5CTL0 &= ~LOCKLPM5;         // Enable the GPIO pins
    P1DIR |= redLED;              // Direct pin as output
    P9DIR |= greenLED;            // Direct pin as output
    P1OUT &= ~redLED;             // Turn LED Off
    P9OUT &= ~greenLED;           // Turn LED Off

    Initialize_UART2();
    config_ACLK_to_32KHz_crystal();

    for(;;)
    {
        for(i = 0; i < 2000; i++) {}
        uart_write_uint16(counter++);
        if (counter == 65536)
            counter = 0;
        uart_write_char('\n');
        uart_write_char('\r');
        for(i = 0; i < 2000; i++) {}
    }
}

// Configure UART to the popular configuration
// 9600 baud, 8-bit data, LSB first, no parity bits, 1 stop bit
// no flow control
// Initial clock: SMCLK @ 1 MHz with oversampling
void Initialize_UART(void){
    // Divert pins to UART functionality
    P3SEL1 &= ~(BIT4|BIT5);
    P3SEL0 |= (BIT4|BIT5);

    // Use SMCLK clock; leave other settings default
    UCA1CTLW0 |= UCSSEL_2;

    // Configure the clock dividers and modulators
    // UCBR=6, UCBRF=8, UCBRS=0x20, UCOS16=1 (oversampling)
    UCA1BRW = 6;
    UCA1MCTLW = UCBRS5|UCBRF3|UCOS16;

    // Exit the reset state (so transmission/reception can begin)
    UCA1CTLW0 &= ~UCSWRST;
}

// Configure UART to the popular configuration
// 4800 baud, 8-bit data, LSB first, no parity bits, 1 stop bit
// no flow control
// Initial clock: SMCLK @ 32 KHz
void Initialize_UART2(void){
    // Divert pins to UART functionality
    P3SEL1 &= ~(BIT4|BIT5);
    P3SEL0 |= (BIT4|BIT5);

    // Use ACLK clock; leave other settings default
    UCA1CTLW0 |= UCSSEL0;

    // Configure the clock dividers and modulators
    //
    UCA1BRW = 6;
    UCA1MCTLW = UCBRS7|UCBRS6|UCBRS5|UCBRS3|UCBRS2|UCBRS1;

    // Exit the reset state (so transmission/reception can begin)
    UCA1CTLW0 &= ~UCSWRST;
}

void uart_write_uint16(unsigned int n){
    unsigned int r[6] = {0};
    int i = 0, j;

    if (n == 0)
        uart_write_char('0');

    while (n != 0)
    {
        r[i++] = n % 10;
        n /= 10;
    }

    for (j = i-1; j >= 0; --j)
        uart_write_char(r[j] + '0');
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
