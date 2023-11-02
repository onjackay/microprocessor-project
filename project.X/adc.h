#ifndef ADC_H
#define	ADC_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <p32xxxx.h>

void initADC( int amask);
int readADC( int ch);


#ifdef	__cplusplus
}
#endif

#endif	/* ADC_H */

