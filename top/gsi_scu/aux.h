#ifndef _AUX_H_
#define _AUX_H_

#include "irq.h"
extern volatile unsigned int* cpu_ID;
extern volatile unsigned int* atomic;

#define F_SYS          62500000ULL
#define T_SYS          1000000000ULL / F_SYS
#define CYCSMICRO      1000ULL/16ULL  

unsigned int irqState;

inline unsigned int  getCpuId();
inline unsigned int  atomic_get();
inline void          atomic_on();   
inline void          atomic_off();

extern volatile unsigned int* cores;
extern volatile unsigned int* time_sys;

inline unsigned int  getCores();
inline unsigned long long get_sys_time();

inline void cycSleep(unsigned int cycs);
inline void uSleep(unsigned long long uSecs);
char* sprinthex(char* buffer, unsigned long val, unsigned char digits);
char* mat_sprinthex(char* buffer, unsigned long val);
#endif
