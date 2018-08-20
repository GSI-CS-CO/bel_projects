#ifndef _DM_H_
#define _DM_H_

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>


#define PRIO_BIT_ENABLE     (1<<0)
#define PRIO_BIT_MSG_LIMIT  (1<<1)
#define PRIO_BIT_TIME_LIMIT (1<<2)

#define PRIO_DAT_STD     0x00
#define PRIO_DAT_TS_HI   0x04
#define PRIO_DAT_TS_LO   0x08
#define PRIO_DRP_TS_HI   0x14
#define PRIO_DRP_TS_LO   0x18

#define DIAG_PQ_MSG_CNT  0x0FA62F9000000000
#define ECA_GLOBAL_ADR   0x7ffffff0

#define PRINT64_FACTOR  1000000000LL

#define SHARED __attribute__((section(".shared")))

#define pDL(x)  (uint64_t*)(x + (T_TD_DEADLINE >> 2))
#define DL(x)   *(pDL(x))
#define pT(x)   (uint32_t*)(*x)
#define pncN(x) (uint32_t*)(pT(x) + (T_TD_NODE_PTR >> 2))
#define pN(x)   (uint32_t*)*pncN(x)


typedef uint64_t  (*deadlineFuncPtr) ( uint32_t*, uint32_t* );
typedef uint32_t* (*nodeFuncPtr)  ( uint32_t*, uint32_t* );
typedef uint32_t* (*actionFuncPtr)( uint32_t*, uint32_t*, uint32_t* );

extern uint8_t cpuId;

extern uint32_t* const _startshared[];
extern uint32_t* const _endshared[];



extern deadlineFuncPtr deadlineFuncs[_NODE_TYPE_END_];
extern nodeFuncPtr         nodeFuncs[_NODE_TYPE_END_];
extern actionFuncPtr     actionFuncs[_ACT_TYPE_END_];
extern uint32_t* const p;
extern uint32_t* const status;
extern uint64_t* const count;
extern uint64_t* const boottime;
#ifdef DIAGNOSTICS
extern int64_t*  const diffsum;
extern int64_t*  const diffmax;
extern int64_t*  const diffmin;
extern int64_t*  const diffwth;
extern uint32_t* const diffwcnt;
extern uint32_t* const diffwhash;
extern uint64_t* const diffwts;
extern uint32_t* const bcklogmax;
extern uint32_t* const badwaitcnt;
#endif
extern uint32_t* const start;
extern uint32_t* const running;
extern uint32_t* const abort1;
extern uint32_t** const hp;     // array of ptrs to threads for scheduler heap



static uint32_t __div64_32(uint64_t *n, uint32_t base)
{
        uint64_t rem = *n;
        uint64_t b = base;
        uint64_t res, d = 1;
        uint32_t high = rem >> 32;

        /* Reduce the thing a bit first */
        res = 0;
        if (high >= base) {
                high /= base;
                res = (uint64_t) high << 32;
                rem -= (uint64_t) (high*base) << 32;
        }

        while ((int64_t)b > 0 && b < rem) {
                b = b+b;
                d = d+d;
        }

        do {
                if (rem >= b) {
                        rem -= b;
                        res += d;
                }
                b >>= 1;
                d >>= 1;
        } while (d);

        *n = res;
        return rem;
}

static char* print64(uint64_t x, int align)
{
        uint32_t h_half, l_half;
        static char buf[2*10+1];        //2x 32-bit value + \0

        if (x < PRINT64_FACTOR)
                if (align)
                        sprintf(buf, "%20u", (uint32_t)x);
                else
                        sprintf(buf, "%u", (uint32_t)x);
        else {
                l_half = __div64_32(&x, PRINT64_FACTOR);
                h_half = (uint32_t) x;
                if (align)
                        sprintf(buf, "%11u%09u", h_half, l_half);
                else
                        sprintf(buf, "%u%09u", h_half, l_half);
        }
        return buf;
}

uint8_t wrTimeValid();

void prioQueueInit();

void dmInit();

uint8_t getNodeType(uint32_t* node);
uint64_t dlEvt(uint32_t* node, uint32_t* thrData);
uint64_t dlBlock(uint32_t* node, uint32_t* thrData);
uint32_t* execNoop(uint32_t* node, uint32_t* cmd, uint32_t* thrData);
uint32_t* execFlow(uint32_t* node, uint32_t* cmd, uint32_t* thrData);
uint32_t* execFlush(uint32_t* node, uint32_t* cmd, uint32_t* thrData);
uint32_t* execWait(uint32_t* node, uint32_t* cmd, uint32_t* thrData);
uint32_t* cmd(uint32_t* node, uint32_t* thrData);
uint32_t* tmsg(uint32_t* node, uint32_t* thrData);
uint32_t* block(uint32_t* node, uint32_t* thrData);
uint32_t* blockFixed(uint32_t* node, uint32_t* thrData);
uint32_t* blockAlign(uint32_t* node, uint32_t* thrData);

uint32_t* nodeNull (uint32_t* node, uint32_t* thrData);
uint64_t  deadlineNull (uint32_t* node, uint32_t* thrData);
uint32_t* dummyNodeFunc (uint32_t* node, uint32_t* thrData);
uint64_t  dummyDeadlineFunc (uint32_t* node, uint32_t* thrData);
uint32_t* dummyActionFunc (uint32_t* node, uint32_t* cmd, uint32_t* thrData);

inline uint32_t hiW(uint64_t dword) {return (uint32_t)(dword >> 32);}
inline uint32_t loW(uint64_t dword) {return (uint32_t)dword;}

void heapify();

void heapReplace(uint32_t src);

#endif