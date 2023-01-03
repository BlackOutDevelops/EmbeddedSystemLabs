#include <msp430FR6989.h>
#define FLAGS      UCA1IFG          // Contains the transmit & receive flags
#define RXFLAG     UCRXIFG          // Receive flag
#define TXFLAG     UCTXIFG          // Transmit flag
#define TXBUFFER   UCA1TXBUF        // Transmit buffer
#define RXBUFFER   UCA1RXBUF        // Receive buffer
#define redLED BIT0                 // Red LED at P1.0
void Initialize_ADC();
void Initialize_UART(void);
void uart_write_char(unsigned char ch);
void uart_write_uint16(unsigned int n);
void uart_write_string(char *str);
// Booster Pack: HOR(X): J1.2 --> Horizontal direction:  Jumper 1 pin 2
// Booster Pack: HOR(Y): J3.26 --> Vertical direction: Jumper 1 pin 26
// Launchpad: Horizontal J1.2:  A10/P9.2 --> Analog input 10 / Port 9.2
// Launchpad: Vertical J3.26:  A4/P8.7 --> Analog input 4 / Port 8.7
// A10 functionality:  P9DIR=x, P9SEL1=1, P9SEL0=1
// A4 functionality:  P8DIR=x, P8SEL1=1, P8SEL0=1
// 10.1 Equation for Sample-and-Hold Time
// (10kOhm + 10kOhm)*(15pF+1pF)*ln(2^(3+2)) = 3 microseconds

int main(void)
{
    volatile unsigned int i;
    unsigned int x = 0, y = 0;
    char xaxis[] = "X-Axis: ", yaxis[] = "Y-Axis: ";
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;         // Enable the GPIO pins
    P1DIR |= redLED;              // Direct pin as output
    P1OUT &= ~redLED;             // Turn LED Off

    Initialize_ADC();
    Initialize_UART();

    while(1)
    {
        ADC12CTL0 |= ADC12SC;

        while (ADC12CTL1 & ADC12BUSY == 1) {}

        x = ADC12MEM0;
        y = ADC12MEM1;

        uart_write_string(xaxis);
        uart_write_uint16(x);
        uart_write_char('\n');
        uart_write_char('\r');
        uart_write_string(yaxis);
        uart_write_uint16(y);
        uart_write_char('\n');
        uart_write_char('\r');
        uart_write_char('\n');
        P1OUT ^= redLED;

        for (i = 0; i < 45000; i++) {}
    }
}

void Initialize_ADC() {
    // Divert the pins to analog functionality
    // X-axis: A10/P9.2, for A10 (P9DIR=x, P9SEL1=1, P9SEL0=1)
    P9SEL1 |= BIT2;
    P9SEL0 |= BIT2;

    // Y-axis: A4/P8.7, for A4 (P8DIR=x, P8SEL1=1, P8SEL0=1)
    P8SEL1 |= BIT7;
    P8SEL0 |= BIT7;

    // Turn on the ADC module
    ADC12CTL0 |= ADC12ON;

    // Turn off ENC (Enable Conversion) bit while modifying the configuration
    ADC12CTL0 &= ~ADC12ENC;

    //***************ADC12CTL0***************
    // Set the bit ADC12MSC (Multiple Sample and Conversion)*
    // Set ADC12SHT0 (select the number of cycles that you determined)
    ADC12CTL0 |= ADC12SHT0_3 | ADC12MSC;

    //***************ADC12CTL1***************
    // Set ADC12CONSEQ (select sequence-of-channels)*
    // Set ADC12SHS (select ADC12SC bit as the trigger)
    // Set ADC12SHP bit
    // Set ADC12DIV (select the divider you determined)
    // Set ADC12SSEL (select MODOSC)
    ADC12CTL1 |= ADC12SHS_0 | ADC12DIV_0 | ADC12SSEL_3 | ADC12SHP | ADC12CONSEQ_1;

    //***************ADC12CTL2***************
    // Set ADC12RES (select 12-bit resolution)
    // Set ADC12DF (select unsigned binary format)
    ADC12CTL2 |= ADC12RES_1;

    //***************ADC12CTL3***************
    // Set ADC12CSTARTADD to 0 (first conversion in ADC12MEM0)*
    // Leave all fields at default values
    ADC12CTL3 |= ADC12CSTARTADD_0;

    //***************ADC12MCTL0***************
    // Set ADC12VRSEL (select VR+=AVCC, VR-=AVSS)
    // Set ADC12INCH (select channel A10)
    ADC12MCTL0 |= ADC12VRSEL_0 | ADC12INCH_10;

    //***************ADC12MCTL1***************
    // Set ADC12VRSEL (select VR+=AVCC, VR-=AVSS)*
    // Set ADC12INCH (select the analog channel that you found)*
    // Set ADC12EOS (last conversion in ADC12MEM1)*
    ADC12MCTL1 |= ADC12VRSEL_0 | ADC12INCH_4 | ADC12EOS;

    // Turn on ENC (Enable Conversion) bit at the end of the configuration
    ADC12CTL0 |= ADC12ENC;
    return;
}

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

void uart_write_string(char *str)
{
    volatile unsigned int i;
    for (i = 0; i<strlen(str); i++)
    {
        uart_write_char(str[i]);
    }
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
