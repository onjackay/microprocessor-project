#include "uart.h"

void initU1( void)
{
    U1BRG = BRATE1;    // initialize the baud rate generator
    U1MODE = U_ENABLE;    // initialize the UART module
    U1STA = U_TX;    // enable the Receiver
//    TRTS2 = 0;    // make RTS an output pin
//    RTS2 = 1;    // set RTS default status (not ready)
} // initU2

void initU2( void)
{
    U2BRG = BRATE2;    // initialize the baud rate generator
    U2MODE = U_ENABLE;    // initialize the UART module
    U2STA = U_RTX;    // enable the Transmitter
    TRISFbits.TRISF5 = 0;
//    TRTS2 = 0;    // make RTS an output pin
//    RTS2 = 1;    // set RTS default status (not ready)
} // initU2

int putU1( int c)
{
//    while ( CTS2);    // wait for !CTS, clear to send
    while ( U1STAbits.UTXBF);    // wait while Tx buffer full
    U1TXREG = c;
    return c;
} // putU1

int putU2( int c)
{
//    while ( CTS2);    // wait for !CTS, clear to send
    while ( U2STAbits.UTXBF);    // wait while Tx buffer full
    U2TXREG = c;
    return c;
} // putU2

int putsU1( char *s ) {
    while (*s != '\0') putU1(*s++);
//    putU1('T'); // end of PWM
}

int putsU2( char *s ) {
    while (*s != '\0') putU2(*s++);
//    putU2('T'); 
}

char getU1( void)
{
//    RTS2=0;    // assert Request To Send !RTS
    while ( !U1STAbits.URXDA); // wait for a new char to arrive
//    RTS2=1;
    return U1RXREG;    // read char from receive buffer
} // getU2

char getU2( void)
{
//    RTS2=0;    // assert Request To Send !RTS
    while ( !U2STAbits.URXDA); // wait for a new char to arrive
//    RTS2=1;
    return U2RXREG;    // read char from receive buffer
} // getU2