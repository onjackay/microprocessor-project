#include "buzzer.h"

// set PWM1 with frequency f and duty cycle k*f
void setPWM1(float f, int k) {
    T2CONbits.ON = 0;
    OC1CONbits.ON = 0;
    // Configure Timer 2
    T2CONbits.TCKPS = 0; // Set prescaler to 1:1
    PR2 = 8000000.0 / f; // Set period register
    TMR2 = 0; // Clear timer register

    // Configure Output Compare 1
    OC1CONbits.OCM = 6; // Set output compare mode to edge-aligned PWM
    OC1CONbits.OCTSEL = 0; // Select Timer 2 as source
//    OC1RS = PR2 * k / 100; // Set duty cycle
    OC1RS = 50; // Set duty cycle
    OC1R = 0; // Initialize OC1R register
    
    T2CONbits.ON = 1; // Turn on Timer 2
    OC1CONbits.ON = 1; // Turn on Output Compare 1
}

void closePWM1() {
    OC1CONbits.ON = 0;
}

void setPWM2(float f, int k) {
    T3CONbits.ON = 0;
    OC2CONbits.ON = 0;
    // Configure Timer 2
    T3CONbits.TCKPS = 0; // Set prescaler to 1:1
    PR3 = 8000000.0 / f; // Set period register
    TMR3 = 0; // Clear timer register

    // Configure Output Compare 1
    OC2CONbits.OCM = 6; // Set output compare mode to edge-aligned PWM
    OC2CONbits.OCTSEL = 1; // Select Timer 3 as source
//    OC2RS = PR2 * k / 100; // Set duty cycle
    OC2RS = 50; // Set duty cycle
    OC2R = 0; // Initialize OC1R register
    
    T3CONbits.ON = 1; // Turn on Timer 2
    OC2CONbits.ON = 1; // Turn on Output Compare 1
}

void closePWM2() {
    OC2CONbits.ON = 0;
}
