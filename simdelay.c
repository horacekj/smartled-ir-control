#include <stdint.h>
#include <pic18.h>

/* delay in 10*x us
 * @16 MHz clock, 0 gives about 6.3 us
 */
void Delay100Us(uint8_t x){   
	uint8_t i;
    for (i=0; i<x; i++){_delay(200);}   // 64 MHz
    for (i=0; i<x; i++){_delay(200);}
    for (i=0; i<x; i++){_delay(200);}
    for (i=0; i<x; i++){_delay(200);}
    for (i=0; i<x; i++){_delay(200);}
    for (i=0; i<x; i++){_delay(200);}
    for (i=0; i<x; i++){_delay(200);}
    for (i=0; i<x; i++){_delay(200);}
};

void DelayMs(uint16_t x){
	for (uint16_t i=0; i<x; i++){Delay100Us(10);}
}


