#include <msp430.h> 
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define FLAGS      UCA1IFG  // Contains the transmit & receive flags
#define RXFLAG     UCRXIFG         // Receive flag
#define TXFLAG     UCTXIFG         // Transmit flag
#define TXBUFFER   UCA1TXBUF       // Transmit buffer
#define RXBUFFER   UCA1RXBUF       // Receive buffer
void Initialize_I2C(void);
void uart_write_string(char *str);
int i2c_read_word(unsigned char i2c_address, unsigned char i2c_reg,unsigned int*data);
int i2c_write_word(unsigned char i2c_address, unsigned char i2c_reg,unsigned int data);
void Initialize_UART(void);
void uart_write_char(unsigned char ch);
unsigned char uart_read_char(void);
void uart_write_intToHex(unsigned int n);
void uart_write_uint16(unsigned int n);
void uart_write_double(double n);

int main(void)
{
    // Configuration(0h01)
    // I2C address of the light sensor is 0b1000100 or 0h44
    // Value of pull up resistors used for I2C lines is 10kOhms
    // DONT FORGET SCREENSHOT IN REPORT!
    unsigned int configuration = 0x7604,
                 result;
    double lux;
    volatile unsigned int i, j, counter = 0;
    char luxString[] = " Lux: ";
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;         // Enable the GPIO pins

    Initialize_I2C();
    Initialize_UART();

    i2c_write_word(0x44, 0x01, configuration);

    for(;;)
    {
        i2c_read_word(0x44, 0x00, &result);
        lux = 1.28*result;

        for(i = 0; i < 2; i++)
        {
            for(j = 0; j < 60000; j++) {}
        }
        uart_write_uint16(counter++);
        uart_write_string(luxString);
        uart_write_uint16(lux);
        uart_write_char('\n');
        uart_write_char('\r');
    }
}

// Configure eUSCI in I2C master mode
void Initialize_I2C(void) {
    // Enter reset state before the configuration starts...
    UCB1CTLW0 |= UCSWRST;

    // Divert pins to I2C functionality
    P4SEL1 |= (BIT1|BIT0);
    P4SEL0 &= ~(BIT1|BIT0);

    // Keep all the default values except the fields below...
    // (UCMode 3:I2C) (Master Mode) (UCSSEL 1:ACLK, 2,3:SMCLK)
    UCB1CTLW0 |= UCMODE_3 | UCMST | UCSSEL_3;

    // Clock divider = 8  (SMCLK @ 1.048 MHz / 8 = 131 KHz)
    UCB1BRW = 8;

    // Exit the reset mode
    UCB1CTLW0 &= ~UCSWRST;
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

void uart_write_intToHex(unsigned int n){
    unsigned int i;
    char hex[4];

    sprintf(hex, "%x", n);

    uart_write_char('0');
    uart_write_char('h');
    for (i = 0; i < strlen(hex); i++)
        uart_write_char(hex[i]);
}

//void uart_write_double(double n){
//    unsigned int i;
//    char stringOfDouble[10];
//
//    fprintf(stringOfDouble, "%e", n);
//
//    for (i = 0; i < strlen(stringOfDouble); i++)
//        uart_write_char(stringOfDouble[i]);
//}

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

void uart_write_string(char *str)
{
    volatile unsigned int i;
    for (i = 0; i<strlen(str); i++)
    {
        uart_write_char(str[i]);
    }
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

// Read a word (2 bytes) from I2C (address, register)
int i2c_read_word(unsigned char i2c_address, unsigned char i2c_reg,unsigned int*data) {
    unsigned char byte1, byte2;

    // Initialize the bytes to make sure data is received every time
    byte1 = 111;
    byte2 = 111;

    //**********Write Frame #1***************************
    UCB1I2CSA = i2c_address;    // Set I2C address
    UCB1IFG &= ~UCTXIFG0;
    UCB1CTLW0 |= UCTR;          // Master writes (R/W bit = Write)
    UCB1CTLW0 |= UCTXSTT;       // Initiate the Start Signal

    while ((UCB1IFG & UCTXIFG0) ==0) {}

    UCB1TXBUF = i2c_reg;        // Byte = register address

    while((UCB1CTLW0 & UCTXSTT)!=0) {}

    if(( UCB1IFG & UCNACKIFG )!=0) return -1;

    UCB1CTLW0 &= ~UCTR;         // Master reads (R/W bit = Read)
    UCB1CTLW0 |= UCTXSTT;       // Initiate a repeated Start Signal
    //****************************************************

    //**********Read Frame #1*****************************
    while ( (UCB1IFG & UCRXIFG0) == 0) {}
    byte1 = UCB1RXBUF;
    //****************************************************

    //**********Read Frame #2*****************************
    while((UCB1CTLW0 & UCTXSTT)!=0) {}
    UCB1CTLW0 |= UCTXSTP;       // Setup the Stop Signal

    while ( (UCB1IFG & UCRXIFG0) == 0) {}
    byte2 = UCB1RXBUF;

    while ( (UCB1CTLW0 & UCTXSTP) != 0) {}
    //****************************************************

    // Merge the two received bytes
    *data = (    (byte1 << 8) | (byte2 & 0xFF)    );
    return 0;
}

// Write a word (2 bytes) to I2C (address, register)
int i2c_write_word(unsigned char i2c_address, unsigned char i2c_reg,unsigned int data) {
    unsigned char byte1, byte2;

    byte1 = (data >> 8) & 0xFF;  // MSByte
    byte2 = data & 0xFF;         // LSByte

    UCB1I2CSA = i2c_address;     // Set I2C address

    UCB1CTLW0 |= UCTR;           // Master writes (R/W bit = Write)
    UCB1CTLW0 |= UCTXSTT;        // Initiate the Start Signal

    while ((UCB1IFG & UCTXIFG0) ==0) {}

    UCB1TXBUF = i2c_reg;        // Byte = register address

    while((UCB1CTLW0 & UCTXSTT)!=0) {}

    //**********Write Byte #1***************************
    UCB1TXBUF = byte1;
    while ( (UCB1IFG & UCTXIFG0) == 0) {}

    //**********Write Byte #2***************************
    UCB1TXBUF = byte2;
    while ( (UCB1IFG & UCTXIFG0) == 0) {}

    UCB1CTLW0 |= UCTXSTP;
    while ( (UCB1CTLW0 & UCTXSTP) != 0) {}

    return 0;
}
