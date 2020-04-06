#ifndef _AUX_H_
#define _AUX_H_

#include <inttypes.h>
#include <stdint.h>
#include "irq.h"

#ifndef __GNUC_STDC_INLINE__
#error NEEDS gnu99 EXTENSIONS - ADD '-std=gnu99' TO THE CFGLAGS OF YOUR Makefile!
#endif

#define false 0   //cant believe I'm doing this ...
#define true  1
//
#define F_SYS          125000000ULL
#define T_SYS          1000000000ULL / F_SYS
#define CYCSMICRO      1000ULL/16ULL  

extern volatile uint32_t* pCpuId;
extern volatile uint32_t* pCpuAtomic;
extern volatile uint32_t* pCluInfo;
extern volatile uint32_t* pCpuSysTime;

uint32_t irqState;

volatile uint32_t ier;

inline uint64_t getSysTime()
{
   uint64_t systime;  
   systime =  ((uint64_t)*(pCpuSysTime+0))<<32;
   systime |= ((uint64_t)*(pCpuSysTime+1)) & 0x00000000ffffffff;
   return systime;  
}

inline void cycSleep(uint32_t cycs)
{
   uint32_t j;
   for (j = 0; j < cycs; ++j) asm("# noop"); 
}

inline void uSleep(uint64_t uSecs)
{
   cycSleep((uint32_t)(uSecs * 1000 / T_SYS));
}

inline uint32_t  getCpuID()  {return *pCpuId;}
inline uint32_t  getCpuIdx() {return *pCpuId   & 0xff;}
inline uint32_t  getCores()  {return *pCluInfo & 0xff;}

inline  uint32_t  atomic_get(void)
{
	 return *pCpuAtomic;	             	
}

static inline void atomic_on()
{
   ier = irq_get_enable();
   irq_disable();
   *pCpuAtomic = 1;
}

static inline void atomic_off()
{
	*pCpuAtomic = 0;
	uint32_t foo = 0; /* UB */
	// or the IE bit with ier
	asm volatile ("rcsr  %0, IE\n"      \
	              "or    %0, %0, %1\n"  \
	              "wcsr  IE, %0\n"      \
                : "+r" (foo)           \
                : "r" (ier)            \
        );        	
}



char progressWheel();
char* sprinthex(char* buffer, unsigned long val, unsigned char digits);
char* mat_sprinthex(char* buffer, unsigned long val);
#endif
