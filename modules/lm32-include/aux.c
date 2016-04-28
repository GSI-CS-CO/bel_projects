#include "aux.h"
#include "irq.h"


/*
inline  unsigned int  atm_get(void)
{
	 //read atomic bit (csr 0x1c)
	 unsigned int atm;
    // gcc doesnt know csr 0x1c, so we must force it (update your f*cking binutils...)
	 asm volatile (	".long 0x93800800" : "=&r" (atm));
    return atm;	             	
}

inline void atomic_on()
{
   //begin atomic operation (hold cycle line on data bus HI)   
   asm volatile ( "mvi r1,1\n" \
                  ".long 0xD3810000" );
   return;	             	
}

inline void atomic_off()
{
	 //end atomic operation (drop cycle line on data bus) 
    asm volatile ( ".long 0xD3800000" );
	 return;	             	
}
*/
extern inline uint64_t  getSysTime();
extern inline void      cycSleep(uint32_t cycs);
extern inline void      uSleep(uint64_t uSecs);
extern inline uint32_t  getCpuID();
extern inline uint32_t  getCpuIdx();
extern inline uint32_t  getCores();

extern inline  uint32_t  atomic_get(void);
extern inline void atomic_on();
extern inline void atomic_off();

char progressWheel()
{
   static unsigned char index;
   const char c_running[4] = {'|', '/', '-', '\\'};
   return c_running[index++ & 0x03];
}

char* sprinthex(char* buffer, unsigned long val, unsigned char digits)
{
	unsigned char i,ascii;
	const unsigned long mask = 0x0000000F;

	for(i=0; i<digits;i++)
	{
		ascii= (val>>(i<<2)) & mask;
		if(ascii > 9) ascii = ascii - 10 + 'A';
	 	else 	      ascii = ascii      + '0';
		buffer[digits-1-i] = ascii;		
	}
	
	buffer[digits] = 0x00;
	return buffer;	
}


char* mat_sprinthex(char* buffer, unsigned long val)
{
   return sprinthex(buffer, val, 8);
}



