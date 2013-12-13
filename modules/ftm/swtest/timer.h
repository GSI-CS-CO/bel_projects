#ifndef _TIMER_H_
#define _TIMER_H_

#define TIMER_ABS_TIME     0
#define TIMER_REL_TIME     1
#define TIMER_PERIODIC     1
#define TIMER_1TIME        0
#define TIMER_NO_CASCADE  -1
#define TIMER_0            0
#define TIMER_1            1
#define TIMER_2            2
#define TIMER_3            3

extern volatile unsigned int* timer;

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











