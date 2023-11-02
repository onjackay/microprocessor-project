#include "adc.h"

void initADC( int amask)
{
    AD1CON1bits.ADON = 0;
    AD1PCFG = amask;    // select analog input pins
    AD1CON1 = 0x00E0;    // automatic conversion after sampling
    AD1CSSL = 0;    // no scanning required
    AD1CON2 = 0;    // use MUXA, use AVdd & AVss as Vref+/-
    AD1CON3 = 0x1F15;    // Tsamp = 32 x Tad; Tad = 44 x 1us; (32 + 12) * 44 * 1 us = 1936 us
    AD1CON1bits.ADON = 1;    // turn on the ADC
} //initADC

int readADC( int ch)
{
    AD1CHSbits.CH0SA = ch;    // 1. select input channel
    AD1CON1bits.SAMP = 1;    // 2. start sampling
    while (!AD1CON1bits.DONE); // 3. wait conversion complete
    return ADC1BUF0;    // 4. read conversion result
} // readADC