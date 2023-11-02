#ifndef BUZZER_H
#define	BUZZER_H

#ifdef	__cplusplus
extern "C" {
#endif
#include <p32xxxx.h>
void setPWM1(float f, int k);
void closePWM1();
void setPWM2(float f, int k);
void closePWM2();

#ifdef	__cplusplus
}
#endif

#endif	/* BUZZER_H */

