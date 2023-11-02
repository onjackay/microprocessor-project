/* 
 * File:   uart.h
 * Author: Lenovo
 *
 * Created on July 24, 2023, 9:15 PM
 */

#ifndef UART_H
#define	UART_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <p32xxxx.h>
    
#define BRATE1 207 // 9600 baud rate, BRGH = 1
#define BRATE2 207
#define U_ENABLE 0x8008 // BRGH = 1
#define U_TX 0x0400
#define U_RX 0x1000
#define U_RTX 0x1400

void initU1( void);
void initU2( void);
int putU1( int c);
int putU2( int c);
int putsU1( char *s );
int putsU2( char *s);
char getU1( void);
char getU2( void);

#ifdef	__cplusplus
}
#endif

#endif	/* UART_H */

