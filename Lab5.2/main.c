// Sample code that implements a stopwatch
#include <msp430fr6989.h>
#define redLED BIT0         // Red at P1.0
#define greenLED BIT7       // Green at P9.7
void Initialize_LCD();
void increment_stopwatch(unsigned int n);
void config_ACLK_to_32KHz_crystal();

// The array has the shapes of the digits (0 to 9)
// Complete this array...39

const unsigned char LCD_Num[10] = {0xFC, 0x60, 0xDB, 0xF3, 0x67, 0xB7, 0xBF, 0xE0, 0xFF, 0xE7};

int main(void) {
    volatile unsigned int n, i=65530;
    WDTCTL = WDTPW | WDTHOLD;       // Stop WDT
    PM5CTL0 &= ~LOCKLPM5;           // Enable GPIO pins
    P1DIR |= redLED;                // Pins as output
    P9DIR |= greenLED;
    P1OUT &= ~redLED;                // Red on
    P9OUT &= ~greenLED;             // Green off

    // Initializes the LCD_C module

    Initialize_LCD();

    // Configure ACLK to the 32 KHz crystal (function call)

    config_ACLK_to_32KHz_crystal();

    // The line below can be used to clear all the segments
    //LCDCMEMCTL = LCDCLRM;         // Clears all the segments

    // Configure Timer_A
    // Timer_A: ACLK, div by 1, up mode, clear TAR

    TA0CTL = TASSEL_1 | ID_0 | MC_1 | TACLR;

    // Ensure flag is cleared at the start

    TA0CTL &= ~TAIFG;

    // Set timer period

    TA0CCR0 = 32738;

    // Infinite loop

    for(;;) {

        // Empty while loop; waits here until TAIFG is raised

        while(!(TA0CTL & TAIFG)) {}

        increment_stopwatch(i++);
        TA0CTL &= ~TAIFG;       // Clear the flag
    }

}

//**********************************************************
// Initializes the LCD_C module
//***Source: Function obtained from MSP430FR6989’s Sample Code***
void Initialize_LCD() {
    PJSEL0 = BIT4 | BIT5;       // For LFXT40
    // Initialize LCD segments 0 - 21; 26 - 43
    LCDCPCTL0 = 0xFFFF;
    LCDCPCTL1 = 0xFC3F;
    LCDCPCTL2 = 0x0FFF;

    // Configure LFXT 32kHz crystal

    CSCTL0_H = CSKEY >> 8;      // Unlock CS registers
    CSCTL4 &= ~LFXTOFF;         // Enable LFXT
    do {
        CSCTL5 &= ~LFXTOFFG;    // Clear LFXT fault flag
        SFRIFG1 &= ~OFIFG;
    }while (SFRIFG1 & OFIFG);   // Test oscillator fault flag
    CSCTL0_H = 0;               // Lock CS registers

    // Initialize LCD_C
    // ACLK, Divider = 1, Pre-divider = 16; 4-pin MUX

    LCDCCTL0 = LCDDIV__1 | LCDPRE__16 | LCD4MUX | LCDLP;

    // VLCD generated internally,
    // V2-V4 generated internally, v5 to ground
    // Set VLCD voltage to 2.60v
    // Enable charge pump and select internal reference for it
    LCDCVCTL = VLCD_1 | VLCDREF_0 | LCDCPEN;

    LCDCCPCTL = LCDCPCLKSYNC;   // Clock synchronization enabled

    LCDCMEMCTL = LCDCLRM;       // Clear LCD memory

    //Turn LCD on

    LCDCCTL0 |= LCDON;
    return;
}

void increment_stopwatch(unsigned int n)
{
    unsigned int r[6], i = 0;
    LCDCMEMCTL = LCDCLRM;

    while (n != 0)
    {
        r[i++] = n % 10;
        n /= 10;
    }

    if (i == 0)
    {
        LCDM8 = LCD_Num[0];
    }
    else if (i == 1)
    {
        LCDM8 = LCD_Num[r[0]];
    }
    else if (i == 2)
    {
        LCDM8 = LCD_Num[r[0]];
        LCDM15 = LCD_Num[r[1]];
    }
    else if (i == 3)
    {
        LCDM8 = LCD_Num[r[0]];
        LCDM15 = LCD_Num[r[1]];
        LCDM19 = LCD_Num[r[2]];
    }
    else if (i == 4)
    {
        LCDM8 = LCD_Num[r[0]];
        LCDM15 = LCD_Num[r[1]];
        LCDM19 = LCD_Num[r[2]];
        LCDM4 = LCD_Num[r[3]];
    }
    else if (i == 5)
    {
        LCDM8 = LCD_Num[r[0]];
        LCDM15 = LCD_Num[r[1]];
        LCDM19 = LCD_Num[r[2]];
        LCDM4 = LCD_Num[r[3]];
        LCDM6 = LCD_Num[r[4]];
    }
    else
    {
        LCDM8 = LCD_Num[r[0]];
        LCDM15 = LCD_Num[r[1]];
        LCDM19 = LCD_Num[r[2]];
        LCDM4 = LCD_Num[r[3]];
        LCDM6 = LCD_Num[r[4]];
        LCDM10 = LCD_Num[r[5]];
    }

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
