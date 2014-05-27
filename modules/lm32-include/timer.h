#ifndef _TIMER_H_
#define _TIMER_H_

extern volatile unsigned int* pCpuTimer;

// Timer IRQ RegisterLayout
static const struct {
   unsigned int rst;
   unsigned int armGet;
   unsigned int armSet;
   unsigned int armClr;
   unsigned int srcGet;
   unsigned int srcSet;
   unsigned int srcClr;
   unsigned int cntModeGet;
   unsigned int cntModeSet;
   unsigned int cntModeClr;
   unsigned int cscGet;
   unsigned int cscSet;
   unsigned int cscClr;
   unsigned int cntRst;
   unsigned int timerSel;
   unsigned int timeHi;
   unsigned int timeLo;
   unsigned int msg;
   unsigned int dstAdr;
   unsigned int cscSel;
   unsigned int cfg_ABS_TIME;
   unsigned int cfg_REL_TIME;
   unsigned int cfg_PERIODIC;
   unsigned int cfg_1TIME;
   unsigned int cfg_NO_CASCADE;
   unsigned int cfg_TIMER_0;
   unsigned int cfg_TIMER_1;
   unsigned int cfg_TIMER_2;
   unsigned int cfg_TIMER_3; 
   
} r_timer = {     .rst        =  0x00 >> 2, // wo, Reset Active Low
                  .armGet     =  0x04 >> 2, // ro, Shows armed timers,  (1 armed, 0 disarmed), 1 bit per pCpuTimer
                  .armSet     =  0x08 >> 2, // wo, arm timers,  
                  .armClr     =  0x0C >> 2, // wo, disarm timers,
                  .srcGet     =  0x10 >> 2, // ro, shows timer sources, (1 counter, 0 time), 1 bit per timer
                  .srcSet     =  0x14 >> 2, // wo, select counter as source
                  .srcClr     =  0x18 >> 2, // wo, select time as source 
                  .cntModeGet =  0x1C >> 2, // ro, shows counter modes, (1 periodic, 0 1time), 1 bit per timer
                  .cntModeSet =  0x20 >> 2, // wo, select periodic mode
                  .cntModeClr =  0x24 >> 2, // wo, select 1time mode
                  .cscGet     =  0x28 >> 2, // ro, shows cascaded start, (1 cascaded, 0 normal), 1 bit per timer 
                  .cscSet     =  0x2C >> 2, // wo, set cascaded start
                  .cscClr     =  0x30 >> 2, // wo, select normal start
                  .cntRst     =  0x34 >> 2, // wo, reset counters, 1 bit per timer
                  .timerSel   =  0x40 >> 2, //timer select. !!!CAUTION!!! content of all regs below depends on this
                  .timeHi     =  0x44 >> 2, // rw, deadline HI word
                  .timeLo     =  0x48 >> 2, // rw, deadline LO word
                  .msg        =  0x4C >> 2, // rw, MSI msg to be sent on MSI when deadline is hit
                  .dstAdr     =  0x50 >> 2, // rw, MSI adr to send the msg to when deadline is hit
                  .cscSel     =  0x54 >> 2,  // rw, select comparator output for cascaded start
                  .cfg_ABS_TIME     = 0,
                  .cfg_REL_TIME     = 1,
                  .cfg_PERIODIC     = 1,
                  .cfg_1TIME        = 0,
                  .cfg_NO_CASCADE   = -1,
                  .cfg_TIMER_0      = 0,
                  .cfg_TIMER_1      = 1,
                  .cfg_TIMER_2      = 2,
                  .cfg_TIMER_3      = 3
                  
};


typedef struct
{
   unsigned char mode;   
   unsigned char src;
            char cascade;   
   unsigned long long deadline;
   unsigned int  msi_dst;
   unsigned int  msi_msg;
} s_timer;

inline void irq_tm_rst(); 

// timer arm
inline unsigned int irq_tm_get_arm(void);
inline void irq_tm_set_arm(unsigned int val);
inline void irq_tm_clr_arm(unsigned int val);
inline void irq_tm_start(unsigned int val);
inline void irq_tm_stop (unsigned int val);

// comparator src
inline unsigned int irq_tm_get_src(void);
inline void irq_tm_set_src(unsigned int val);
inline void irq_tm_clr_src(unsigned int val);

// counter mode
inline unsigned int irq_tm_get_cnt_mode(void);
inline void irq_tm_set_cnt_mode(unsigned int val);
inline void irq_tm_clr_cnt_mode(unsigned int val);

// cascaded start
inline unsigned int irq_tm_get_csc(void);
inline void irq_tm_set_csc(unsigned int val);
inline void irq_tm_clr_csc(unsigned int val);

// counter reset
inline void irq_tm_cnt_rst(unsigned int val);

//timer select
inline void irq_tm_timer_sel(unsigned int val);

//deadline
inline void irq_tm_deadl_set(unsigned long long deadline);
unsigned long long irq_tm_deadl_get(unsigned char idx);

//irq address and message
inline void irq_tm_msi_set(unsigned int dst, unsigned int msg);

//cascade counter 'reciever' to timer 'sender'. sender -1 means no cascade
inline void irq_tm_cascade(char receiver, char sender);

void irq_tm_trigger_at_time(unsigned char idx, unsigned long long deadline);

void irq_tm_write_config(unsigned char idx, s_timer* tm);

#endif











