#include "main.h"
#include "buzzer.h"
#include "adc.h"
#include "uart.h"
#include <math.h>
#include <proc/p32mx795f512l.h>

int main(int argc, char** argv) {
    initMCU();
    int i=0, _=0, x=0, y=0;
    int b=0, c=0;
    int lstx=0;
    char str[50];
    bits[0] = 1;
    for (i=1;i<16;i++) {
        bits[i]=bits[i-1]<<1; buzId[i]=-1;
    }
    for (i=0;i<7;i++) used[i]=0;
    initADC(0xfffe);
    initU1();
    initU2();
    
    set_PWM_duty(1, 0);
    DelayMsec(50);
    set_PWM_duty(2, 0);
    DelayMsec(50);
    
    a = readADC(0);
    tune = a / 85;
    lsttune = tune;
    
    DelayMsec(1000);
    
    putsU2("start=1\xff\xff\xff");
    putsU2("tune=");
    itoa(str, tune, 10);
    putsU2(str);
    putsU2("\xff\xff\xff");
    
    U2STAbits.URXDA = 0;
    
    while (1) {
        x = PORTE & (0xff);
        y = (PORTD >> 2) & (0x3f) | (PORTDbits.RD9 << 6);
        x = x | (y << 8);
        
        if (x!=lstx) {
            for (i=0;i<15;i++) {
                if ((x&bits[i])&& !(lstx&bits[i])) {
                    start(i, F[G[14-i]+tune]);
                }
                else if (!(x&bits[i])&& (lstx&bits[i])) {
                    stop(i);
                }
            }
            lstx = x;
            itoa(str, x, 10);
            putsU2("come=1\xff\xff\xff");
            putsU2("give=");
            putsU2(str);
            putsU2("\xff\xff\xff");
        }
        
        a = readADC(0);
        tune = a / 86;
        if (tune != lsttune) {
            putsU2("tune=");
            itoa(str, tune, 10);
            putsU2(str);
            putsU2("\xff\xff\xff");
            lsttune = tune;
        }
        
        if (U2STAbits.URXDA) {
            b = U2RXREG;
            if (c<5) {c++; continue;}
            if (b==0x0) play1();
            else if (b==0x01) play2();
            else if (b==0x10) play3();
            else if (b==0x11) play4();
        }
    }
    return (EXIT_SUCCESS);
}

void initMCU(void) {

    /* setup Timer to count for 1 us and 1 ms */
    SYSKEY = 0x0;               // Ensure OSCCON is lock
    SYSKEY = 0xAA996655;        // Unlock sequence part 1 of 2 back to back instructions.
    SYSKEY = 0x556699AA;        // Unlock sequence part 2 of 2 back to back instructions.
    OSCCONbits.NOSC = 0x0007;   // Write new osc src value to NOSC control bits -- FRS, with original freqeuncy as 8 MHz
    OSCCONbits.FRCDIV = 0x0; // the prescale of FRC is 1
    OSCCONbits.PBDIV = 0x0;    // PBCLK is SYSCLK divided by 1. {(Not changed here)Clock is multiplied by 15. PLL output is divided by 1} -- PBCLK has freqeuncy 8 MHz
    OSCCONbits.OSWEN = 0x0001;  // Initiate clock switch by setting OSWEN bit.
    SYSKEY = 0x0;               // Write non-key value to perform a re-lock.

    while(OSCCONbits.OSWEN);    // Loop until OSWEN = 0. Value of 0 indicates osc switch is complete.
    
    T1CON = 0x0;
    PR1 = 0xFFFF;
    T1CONSET = 0x8000;
    /* Configure Timer interrupts */ 
    INTCONbits.MVEC = 1;  // multi-vector mode
    IPC1SET = 0x000d; // timer 1: priority is 3, subpriority is 1
    // IPC2SET = 0x000d;  // timer 2: priority is 3, subpriority is 1
    IFS0CLR = 0x0110;  // clear the flags for timer 1 and timer 2
    /* enable global and individual interrupts */
    asm( "ei" );  // enable interrupt globally
    IEC0SET = 0x0110; // enable interrupt for timer 1 and timer 2
}

/* configure timer SFRs to generate num us delay*/
void DelayUsec(int num) {
    PR1 = num << 3;
    flags.timer1_done = 0;
    T1CONSET = 0x8000;              // turn on the timer 1
    TMR1 = 0x0;                     // reset the timer 1
    while ( ! flags.timer1_done );  // loop until flag 04 (for timer 1) is set
    flags.timer1_done = 0;          // reset the flags
    T1CONCLR = 0x8000;                    // turn off timer 
}

/* configure timer SFRs to generate 1 ms delay*/
void GenMsec() {
    DelayUsec(1000);
}

/* Call GenMsec() num times to generate num ms delay*/
void DelayMsec(int num) {
    int i;
    for (i=0; i<num; i++) {
        GenMsec();
    }
}

/* timer 1 interrupt handler */ 
#pragma interrupt timer_1_interrupt ipl3 vector 4
void timer_1_interrupt(void) {
    T1CONCLR = 0x8000;                // turn off the timer 1
    IFS0CLR = 0x0010;           // clear the flag for timer 1
    flags.timer1_done = 1;      // set up the flags
    TMR1 = 0;
}

/* initialize the LCD module */
void LCD_init() {
    TRISDbits.TRISD2 = 0;
    TRISDbits.TRISD3 = 0;
    TRISECLR = 0xFFFF;
    
    DelayMsec(16);      //wait for 16 ms(over 15ms)
    RS = 0;             //send command
    E = 1;
    Data = LCD_IDLE;    //function set - 8 bit interface
    E = 0;
    DelayMsec(5);       //wait for 5 ms
    E = 1;
    Data = LCD_IDLE;    //function set - 8 bit interface
    E = 0;
    DelayUsec(200);     //wait for 200 us(over 100us)
    E = 1;
    Data = LCD_IDLE;    //function set
    E = 0;
    DelayMsec(5);
    E = 1;
    Data = 0b00100000; //function set - 4 bit interface
    E = 0;
    LCD_putchar(LCD_2_LINE_4_BITS);
    DelayMsec(100);
    LCD_putchar(LCD_DSP_CSR);
    DelayUsec(400);
    LCD_putchar(LCD_CLR_DSP);
    DelayMsec(5);
    LCD_putchar(LCD_CSR_INC);
}

/* Send one byte c (instruction or data) to the LCD */
void LCD_putchar(unsigned char c) {
    E = 1;
    Data = c;   //sending higher nibble
    E = 0;      //producing falling edge on E
    E = 1;
    Data <<= 4; //sending lower nibble through higher 4 ports
    E = 0;      //producing falling edge on E
    DelayMsec(1);
}

/* Display a string of characters *s by continuously calling LCD_putchar() */
void LCD_puts(const unsigned char *s, int length) {
    // ...NOTE: must wait for at least 40 us after sending each character to
        // the LCD module.
    RS = 1;
    int i;
    for (i = 0; i < length; i++) {
        LCD_putchar(s[i]);
        DelayMsec(30);
    }
    RS = 0;
}

/* go to a specific DDRAM address addr */
void LCD_goto(int addr) {
    // ...send an address to the LCD
    // ...NOTE: must wait for at least 40 us after the address is sent
    RS = 0;
    E = 1; Data = 0x80 + addr; E = 0;
    E = 1; Data <<=4;          E = 0;
    DelayUsec(40);
}

// k = 1 or 2.
void set_PWM_freq(int k, float f) {
    putsU1(k == 1 ? "S1F" : "S2F");
    if (f < 1000) {
        int g = (int)(f+0.5);
        int a = g/100, b = g/10%10, c = g%10;
        putU1('0' + a); putU1('0' + b); putU1('0' + c);
    }
    else {
        f/=100;
        int g = (int)(f+0.5);
        int a = g/100, b = g/10%10, c = g%10;
        putU1('0' + a); putU1('0' + b); putU1('.'); putU1('0' + c);
    }
    putU1('T');
}

// k = 1 or 2, 1 <= d <= 100
void set_PWM_duty(int k, int d) {
    putsU1(k == 1 ? "S1D" : "S2D");
    int a = d/100, b = d/10%10, c = d%10;
    putU1('0' + a); putU1('0' + b); putU1('0' + c);
    putU1('T');
}

void DelaySec(int num) {
    int i;
    for (i=0; i< num * 1000; i++) {
        GenMsec();
    }
}

// shanhuhai
void play1() {
    int P1[100] = {
        G4, D5, D5, E5, 
        E5, F5, E5, C5, A5, G5,
        G5, A5, B5, D6, D6, C6, 
        C6, B5, A5, B5, A5, G5,
        -1, G5, B5, B5, C6,
        C6, B5, A5, B5, A5, G5,
        -1, E5, A5, A5, C5, 
        A5, G5, E5, A5, A5, G5
    };
    float Q1[100] = {
        1, 1, 1, 3,
        1, 1, 1, 1, 1.5, 2.5,
        1, 1, 1, 1, 1.5, 2.5,
        1, 1, 1, 1, 1.5, 2.5,
        2, 1, 1, 1, 3, 
        1, 1, 1, 1, 1.5, 2.5,
        2, 1, 1, 1, 3,
        1, 1, 1, 1, 4, 2
    };
    int R1[100] = {
        4, 1, 1, 2,
        2, 3, 2, 0, 5, 4,
        4, 5, 6, 8, 8, 7,
        7, 6, 5, 6, 5, 4,
        -1, 4, 6, 6, 7,
        7, 6, 5, 6, 5, 4,
        -1, 2, 5, 5, 0,
        5, 4, 2, 5, 5, 4
    };
    char str[50];
    int i, x=0;
    for (i=0; i<44; i++) {
        a = readADC(0);
        tune = a / 86;
        if (tune != lsttune) {
            putsU2("tune=");
            itoa(str, tune, 10);
            putsU2(str);
            putsU2("\xff\xff\xff");
            lsttune = tune;
        }
        if (P1[i] >= 0) {
            putsU2("come=1\xff\xff\xff");
            putsU2("give=");
            x = 1<<(14 - R1[i]);
            itoa(str, x, 10);
            putsU2(str);
            putsU2("\xff\xff\xff");
            setPWM1(F[P1[i]+tune], 100);
            setPWM2(F[P1[i]+tune], 100);
        }
        else {
            putsU2("come=1\xff\xff\xff");
            putsU2("give=0\xff\xff\xff");
            closePWM1();
            closePWM2();
        }
        DelayMsec(400.0*Q1[i]);
    }
    closePWM1();
            closePWM2();
            putsU2("come=1\xff\xff\xff");
            putsU2("give=0\xff\xff\xff");
    putsU2("return=1\xff\xff\xff");
}

// shuanghua
void play2() {
    int P1[100] = {
        A4, C5, C5, A4, D5, E5, D5,
        E5, E5, G4, A4, -1,
        A4, C5, C5, A4, D5, E5, D5,
        E5, D5, G5, E5, E5, G5, 
        A5, C5, E5, D5, C5, D5,
        E5, A4, C5, A4, A4, G4,
        A4, E5, D5, E5, D5, C5, B4, A4
    };
    float Q1[100] = {
        1, 1, 1, 1, 1, 1, 2,
        1, 2, 1, 2, 2,
        1, 1, 1, 1, 1, 1, 2,
        2, 1, 1, 2, 1, 1, 
        2, 1, 1, 2, 1, 1,
        2, 1, 1, 2, 1, 1,
        3, 1, 1, 0.5, 0.5, 1, 1.5, 3.5
    };
    int R1[100] = {
        5, 7, 7, 5, 8, 9, 8,
        9, 9, 4, 5, -1,
        5, 7, 7, 5, 8, 9, 8,
        9, 8, 11, 9, 9, 11,
        12, 7, 9, 8, 7, 8,
        9, 5, 7, 5, 5, 4,
        5, 9, 8, 9, 8, 7, 6, 5
    };
    char str[50];
    int i, x=0;
    for (i=0; i<45; i++) {
        a = readADC(0);
        tune = a / 86;
        if (tune != lsttune) {
            putsU2("tune=");
            itoa(str, tune, 10);
            putsU2(str);
            putsU2("\xff\xff\xff");
            lsttune = tune;
        }
        if (P1[i] >= 0) {
            putsU2("come=1\xff\xff\xff");
            putsU2("give=");
            x = 1<<(14 - R1[i]);
            itoa(str, x, 10);
            putsU2(str);
            putsU2("\xff\xff\xff");
            setPWM1(F[P1[i]+tune], 100);
            setPWM2(F[P1[i]+tune], 100);
        }
        else {
            putsU2("come=1\xff\xff\xff");
            putsU2("give=0\xff\xff\xff");
            closePWM1();
            closePWM2();
        }
        DelayMsec(350.0*Q1[i]);
    }
    closePWM1();
            closePWM2();
            putsU2("come=1\xff\xff\xff");
            putsU2("give=0\xff\xff\xff");
    putsU2("return=1\xff\xff\xff");
}

// yaya
void play3() {
    int P1[100] = {
        E4, E4, G4, C5, B4, A4, A4,
        A4, G4, E4, D4, E4, E4,
        E4, G4, D4, C4, D4, C4,
        D4, C4, D4, G4, E4, E4,
        E4, G4, C5, B4, A4, A4,
        A4, E5, D5, C5, G4, G4,
        G4, E4, G4, A4, B4, B4,
        B4, C5, B4, A4, B4
    };
    float Q1[100] = {
        1, 1, 1, 1, 1, 3, 1,
        1, 1, 1, 1, 3, 1,
        1, 1, 1, 1, 3, 1,
        1, 1, 1, 1, 3, 1,
        1, 1, 1, 1, 3, 1,
        1, 1, 1, 1, 3, 1,
        1, 1, 1, 1, 3, 1,
        1, 1, 1, 1, 4
    };
    int R1[100] = {
        2, 2, 4, 7, 6, 5, 5,
        5, 4, 2, 1, 2, 2,
        2, 4, 1, 0, 1, 0,
        1, 0, 1, 4, 2, 2,
        2, 4, 7, 6, 5, 5,
        5, 9, 8, 7, 4, 4,
        4, 2, 4, 5, 6, 6,
        6, 7, 6, 5, 6
    };
    char str[50];
    int i, x=0;
    for (i=0; i<48; i++) {
        a = readADC(0);
        tune = a / 86;
        if (tune != lsttune) {
            putsU2("tune=");
            itoa(str, tune, 10);
            putsU2(str);
            putsU2("\xff\xff\xff");
            lsttune = tune;
        }
        if (P1[i] >= 0) {
            putsU2("come=1\xff\xff\xff");
            putsU2("give=");
            x = 1<<(14 - R1[i]);
            itoa(str, x, 10);
            putsU2(str);
            putsU2("\xff\xff\xff");
            setPWM1(F[P1[i]+tune], 100);
            setPWM2(F[P1[i]+tune], 100);
        }
        else {
            putsU2("come=1\xff\xff\xff");
            putsU2("give=0\xff\xff\xff");
            closePWM1();
            closePWM2();
        }
        DelayMsec(270.0*Q1[i]);
    }
    closePWM1();
            closePWM2();
            putsU2("come=1\xff\xff\xff");
            putsU2("give=0\xff\xff\xff");
    putsU2("return=1\xff\xff\xff");
}

// ltx
void play4() {
    int P1[100] = {
        G4, A4, C5, D5, C5, D5, E5, D5, 
        C5, -1, C5, D5, E5, G5, E5, D5, C5, A4, G4,
        E5, -1, E5, G5, A5, E5, D5, D5, C5, E5, D5,
        C5, -1, A4, C5, E5, D5, C5, A4, G4, E5, D5, C5
    };
    float Q1[100] = {
        1, 1, 1, 3, 1, 2, 1, 1,
        4.5, 0.5, 1, 1, 1, 3, 1, 1, 1, 1, 1, 
        4.5, 0.5, 1, 1, 1, 3, 1, 1, 1, 1, 1,
        4.5, 0.5, 1, 1, 1, 3, 1, 1, 1, 1, 1, 8
    };
    int R1[100] = {
        4, 5, 7, 8, 7, 8, 9, 8, 
        7, -1, 7, 8, 9, 11, 9, 8, 7, 5, 4,
        9, -1, 9, 11, 12, 9, 8, 8, 7, 9, 8,
        7, -1, 5, 7, 9, 8, 7, 5, 4, 9, 8, 7
    };
    char str[50];
    int i, x=0;
    for (i=0; i<42; i++) {
        a = readADC(0);
        tune = a / 86;
        if (tune != lsttune) {
            putsU2("tune=");
            itoa(str, tune, 10);
            putsU2(str);
            putsU2("\xff\xff\xff");
            lsttune = tune;
        }
        if (P1[i] >= 0) {
            putsU2("come=1\xff\xff\xff");
            putsU2("give=");
            x = 1<<(14 - R1[i]);
            itoa(str, x, 10);
            putsU2(str);
            putsU2("\xff\xff\xff");
            setPWM1(F[P1[i]+tune], 100);
            setPWM2(F[P1[i]+tune], 100);
        }
        else {
            putsU2("come=1\xff\xff\xff");
            putsU2("give=0\xff\xff\xff");
            closePWM1();
            closePWM2();
        }
        DelayMsec(350.0*Q1[i]);
    }
    closePWM1();
            closePWM2();
            putsU2("come=1\xff\xff\xff");
            putsU2("give=0\xff\xff\xff");
    putsU2("return=1\xff\xff\xff");
}

int start(int k, float f) {
    if (buzId[k]!=-1) return -1;
    if (f < 1000) {
        if (!used[2]) {
            set_PWM_freq(1, f);
            DelayMsec(50);
            set_PWM_duty(1, 70);
            DelayMsec(50);
            buzId[k]=2;
            used[2]=1;
            return 0;
        }
        else if (!used[3]) {
            set_PWM_freq(2, f);
            DelayMsec(50);
            set_PWM_duty(2, 70);
            DelayMsec(50);
            buzId[k]=3;
            used[3]=1;
            return 0;
        }
    }
    if (!used[0]) {
        setPWM1(f, 1);
        buzId[k]=0;
        used[0]=1;
    }
    else if (!used[1]) {
        setPWM2(f, 1);
        buzId[k]=1;
        used[1]=1;
    }
}

void stop(int k) {
    used[buzId[k]]=0;
    if (buzId[k]==0) {
        closePWM1();
    }
    else if (buzId[k]==1) {
        closePWM2();
    }
    else if (buzId[k]==2) {
        set_PWM_duty(1, 0);
        DelayMsec(50);
    }
    else if (buzId[k]==3) {
        set_PWM_duty(2, 0);
        DelayMsec(50);
    }
    buzId[k]=-1;
}