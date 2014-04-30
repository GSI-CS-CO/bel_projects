#ifndef _AUX_H_
#define _AUX_H_

#define false 0   //cant believe I'm doing this ...
#define true  1

#define F_SYS          62500000ULL
#define T_SYS          1000000000ULL / F_SYS
#define CYCSMICRO      1000ULL/16ULL  

extern volatile unsigned int* pCpuId;
extern volatile unsigned int* pCpuAtomic;
extern volatile unsigned int* pCluInfo;
extern volatile unsigned int* pCpuSysTime;

unsigned int irqState;

inline unsigned int  getCpuId();
inline unsigned int  atomic_get();
inline void          atomic_on();   
inline void          atomic_off();

inline unsigned int  getCores();
inline unsigned long long getSysTime();

inline void cycSleep(unsigned int cycs);
inline void uSleep(unsigned long long uSecs);

char progressWheel();
char* sprinthex(char* buffer, unsigned long val, unsigned char digits);
char* mat_sprinthex(char* buffer, unsigned long val);
#endif
