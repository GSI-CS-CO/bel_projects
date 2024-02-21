#ifndef _DM_H_
#define _DM_H_

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>

/** @name Priority Queue - mode register
 *  Bit definitions for the Mode register of the Priority Queue 
 */
//@{
//#define USE_SW_TX_CONTROL 1      ///< Use EBM software access instead of hardware priority queue module
#define PRIO_BIT_ENABLE     (1<<0) ///< global enable bit
#define PRIO_BIT_MSG_LIMIT  (1<<1) ///< message limit enable bit
#define PRIO_BIT_TIME_LIMIT (1<<2) ///< time limit enable bit
//@}

/** @name Priority Queue - message tags
 *  data tags for message FIFO input of the Priority Queue  
 */
//@{ 
#define PRIO_DAT_STD     0x00 ///< Address based tag for Priority Queue input FIFO. standard data
#define PRIO_DAT_TS_HI   0x04 ///< Address based tag for Priority Queue input FIFO. timestamp high word
#define PRIO_DAT_TS_LO   0x08 ///< Address based tag for Priority Queue input FIFO. timestamp low word
#define PRIO_DRP_TS_HI   0x14 ///< Address based tag for Priority Queue input FIFO. drop msg request bit set, timestamp high word
#define PRIO_DRP_TS_LO   0x18 ///< Address based tag for Priority Queue input FIFO. drop msg request bit set, timestamp low word
//@}

/** @name Priority Queue - Configuration
 *  Magic words and predefined address values for PQ configuration  
 */
//@{ 
/** Magic word for diagnostics. Causes priority queue global message count to overwrite timing message parameter
 * field before sending the message. This makes current global msg count available at listening timing receivers
 */
#define DIAG_PQ_MSG_CNT  0x0FA62F9000000000 
#define ECA_GLOBAL_ADR   0x7ffffff0 ///< Hardcoded address of ECA unit at all timing endpoints for EB broadcast
//@}


#define PRINT64_FACTOR  1000000000LL

#define SHARED __attribute__((section(".shared")))
#define PEER_ADR_MSK        (RAM_SIZE-1) //FIXME this is built on two questionable assumptions: 1. RAM_SIZE is a power of 2 2. all ram sizes are equal

/** @name Shorthands for accessing node, deadline and thread of heap elements
 *  Used to get the node with earliest deadline plus its working thread and update deadline afterwards in main EDF loop
 */
//@{ 
#define pDL(x)  (uint64_t*)(x + (T_TD_DEADLINE >> 2))     ///< ptr to deadline of thread at heap element x
#define DL(x)   *(pDL(x))                                 ///< deadline of of thread at heap element x
#define pT(x)   (uint32_t*)(*x)                           ///< ptr to thread at heap element x
#define pncN(x) (uint32_t*)(pT(x) + (T_TD_NODE_PTR >> 2)) ///< auxiliary (pN does not compile everywhere, don't ask why. Need to use *pncN instead)
#define pN(x)   (uint32_t*)*pncN(x)                       ///< ptr to node at the cursor of the thread of heap element x
//@}

/** @name Hardware CPU Attributes
 *  The CPU identifier (Index number) and the total available CPU cores of the DM
 */
//@{ 
extern uint8_t cpuId; ///< Id number of this cpu
extern uint8_t cpuQty; ///< total number of cpus on the DM
//@}

/** @name Firmware thread and scheduler attributes
 *  Max number of threads affect memory layout, thus hardcoded
 */
//@{
//#define _THR_QTY_               32           ///< Maximum number of threads
//#define _HEAP_SIZE_             (_THR_QTY_) ///< Scheduler heap size (power of 2)
//@}


/** @name Shorthands for accessing shared memory area
 *  Provides a shorthand to the start and end of the shared memory area used for host/inter cpu communication
 */
//@{ 
extern uint32_t* const _startshared[]; ///< ptr to start of shared memory area
extern uint32_t* const _endshared[];   ///< ptr to end of shared memory area
extern uint32_t* const p;              ///< the short shortcut to the start of shared memory area, basis for most ptr arithmetic
//@}

/** @name Function ptr arrays to node/deadline/action handlers
 *  Provides a shorthand to the handler functions for different node types, next deadline calculation and command action execution
 */
//@{ 
typedef uint64_t  (*deadlineFuncPtr) ( uint32_t*, uint32_t* );
typedef uint32_t* (*nodeFuncPtr)  ( uint32_t*, uint32_t* );
typedef uint32_t* (*actionFuncPtr)( uint32_t*, uint32_t*, uint32_t* );
extern deadlineFuncPtr deadlineFuncs[_NODE_TYPE_END_];  ///< Function pointer array to deadline generating Functions
extern nodeFuncPtr         nodeFuncs[_NODE_TYPE_END_];  ///< Function pointer array to node handler functions
extern actionFuncPtr      actionFuncs[_ACT_TYPE_END_];  ///< Function pointer array to command action handler functions
//@}

/** @name Compilation of nodes referencing fields from others during runtime.
 *  This is in fact not references but done by value copy, the node handler function is always given a 'static' node to work with. Thus, this is RO, changes will not propagate back.
 */
//@{ 
extern uint32_t              nodeTmp[_MEM_BLOCK_SIZE / _32b_SIZE_]; ///< Staging area when a node is constructed from references
extern uint32_t* dynamicNodeStaging(uint32_t* node, uint32_t* thrData);     ///< Returns ptr to the original node if all fields are immediates or ptr to nodeTmp if a dynamic verion was compiled
//@}

/** @name Ptrs to diagnostic data
 *  Provides a shorthand to the diagnostic buffers for msg count, dispatch delta, late warnings, backlog etc
 */
//@{ 
extern uint32_t* const status;          ///< ptr to status register
extern uint64_t* const count;           ///< ptr to global message count register        
extern uint64_t* const boottime;        ///< ptr to bootime registers
//#ifdef DIAGNOSTICS
extern int64_t*  const diffsum;         ///< ptr to dispatch delta sum
extern int64_t*  const diffmax;         ///< ptr to dispatch delta max
extern int64_t*  const diffmin;         ///< ptr to dispatch delta min        
extern int64_t*  const diffwth;         ///< ptr to dispatch delta warning threshold
extern uint32_t* const diffwcnt;        ///< ptr to dispatch delta warning count
extern uint32_t* const diffwhash;       ///< ptr to dispatch delta warning node hash of 1st occurrence
extern uint64_t* const diffwts;         ///< ptr to dispatch delta warning timestamp of 1st occurrence
extern uint32_t* const backlogmax;       ///< ptr to backlog max
extern uint32_t* const badwaitcnt;      ///< ptr to bad waittime count
//#endif
//@}

/** @name Ptrs to scheduler control registers (start, running, abort)
 *  Provides a shorthand to the scheduler's start, running and abort registers used for thread control
 */
//@{ 
extern uint32_t* const start;           ///< ptr to thread control - start bits
extern uint32_t* const running;         ///< ptr to thread control - running bits
extern uint32_t* const abort1;          ///< ptr to thread control - abort bits (name awkwardly chosen to avoid clash with WR global)
extern uint32_t** const hp;             ///< ptr array of EDF scheduler heap
//@}

/** @name Auxiliary functions
 *  Formatting for large numbers & word extraction
 */
//@{ 
/// Divides 64b value by base
/** Divides 64b value by base */
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

/// print function for 64b values to console
/** Does the division necessary to show 64b values in decimal */
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

inline uint32_t hiW(uint64_t dword) {return (uint32_t)(dword >> 32);} ///< Returns high word of 64b value
inline uint32_t loW(uint64_t dword) {return (uint32_t)dword;} ///< Returns low word of 64b value

inline uint8_t hasNodeDynamicFields(uint32_t* node) {
  return (node[NODE_OPT_DYN  >> 2] > 0);
}


/// Validate WR time
/** Checks WR module status bits for valid PPS signal and timestamp  
 * @return 1 if WR time is valid, 0 if not
 */
uint8_t wrTimeValid();

/// Initialiser for the priority queue
/** Init priority queue so it knows where to find the ECA and set message and time limits */
void prioQueueInit();

/// Initialiser for the data master
/** Init data master function pointer arrays. Clear heap, thread control data and diagnostic infos. */
void dmInit();

/// Get node type
/** Returns the 4 bit type value of a node
  * @param node Pointer to the data node whose type is to be determined
  * @return type field of node if valid, type NULL if node is a null ptr or type UNKNOWN if type is not listed
  */
uint8_t getNodeType(uint32_t* node);

/// Calculates next deadline for a node of basetype event (tmsg, cmd, switch)
/** Returns the 64 bit deadline for this event node by adding its offset to the thread time sum
  * @param node Pointer to the evt basetype node whose deadline is to be determined
  * @param thrData Pointer to the associated thread's metadata
  * @return 64b TAI timestamp with new deadline
 */
uint64_t dlEvt(uint32_t* node, uint32_t* thrData);

/// Calculates next deadline for a node of basetype block
/** Returns the 64 bit deadline for this block node which is the thread time sum 
  * @param node Pointer to the block basetype node whose deadline is to be determined
  * @param thrData Pointer to the associated thread's metadata
  * @return 64b TAI timestamp with new deadline
  */
uint64_t dlBlock(uint32_t* node, uint32_t* thrData);

/// Executes a command action of type Noop
/** Noops only effect is the return of the target blocks' default successor as next node to process
  * @param node Pointer to current block
  * @param cmd Pointer to the command to be executed within current block's queues
  * @param thrData Pointer to the associated thread's metadata
  * @return Ptr to successor node
*/
uint32_t* execNoop(uint32_t* node, uint32_t* cmd, uint32_t* thrData);

/// Executes a command action of type Flow
/** Returns the destination of the flow command as next node to process. If action is permanent, destination also overwrites to target blocks' default successor
  * @param node Pointer to current block
  * @param cmd Pointer to the command to be executed within current block's queues
  * @param thrData Pointer to the associated thread's metadata
  * @return Ptr to hosting block's default successor node
*/
uint32_t* execFlow(uint32_t* node, uint32_t* cmd, uint32_t* thrData);

/// Executes a command action of type Flush
/** Clears zero or more queues of target block. Returns target's default successor  as next node to process
  * @param node Pointer to current block
  * @param cmd Pointer to the command to be executed within current block's queues
  * @param thrData Pointer to the associated thread's metadata
  * @return Ptr to flow destination (block's default successor node or one from its alternative destinations list)
*/
uint32_t* execFlush(uint32_t* node, uint32_t* cmd, uint32_t* thrData);

/// Executes a command action of type Wait
/** If relative, wait extends target block's period by its own value. If absolute, wait extends target block's period until its own timestamp is reached. Returns target's default successor as next node to process
  * @param node Pointer to current block
  * @param cmd Pointer to the command to be executed within current block's queues
  * @param thrData Pointer to the associated thread's metadata
  * @return Ptr to hosting block's default successor node or overrride destination if sppecified
*/
uint32_t* execWait(uint32_t* node, uint32_t* cmd, uint32_t* thrData);

/// Dispatches the action of command node in the schedule to its target
/** Writes action (noop, flush, flow ...) of a command node to its target node (can be on a different CPU). For this, the current buffer element must be selected
  * @param node Pointer to current node of basetype command. Its action will be dispatched to its target block
  * @param thrData Pointer to the associated thread's metadata
  * @return Ptr to hosting block's default successor node
*/
uint32_t* cmd(uint32_t* node, uint32_t* thrData);

/// Processes a switch node, changing a target's successor
/** Overwrites target's successor. No queueing involved, effect is immediate.
  * @param node Pointer to current node of basetype switch
  * @param thrData Pointer to the associated thread's metadata
  * @return Ptr to successor node
*/
uint32_t* cswitch(uint32_t* node, uint32_t* thrData);


/// Processes an origin node, changing the origin of a given cpu/thr combo to given node adr
/** Overwrites target's successor. No queueing involved, effect is immediate.
  * @param node Pointer to current node of basetype origin
  * @param thrData Pointer to the associated thread's metadata
  * @return Ptr to successor node
*/
uint32_t* origin(uint32_t* node, uint32_t* thrData);

/// Processes a startthread node 
/** Overwrites target's successor. No queueing involved, effect is immediate.
  * @param node Pointer to current node of basetype origin
  * @param thrData Pointer to the associated thread's metadata
  * @return Ptr to successor node
*/
uint32_t* startThread(uint32_t* node, uint32_t* thrData);

/// Generates a timing message and dispatches it to priority queue
/** Reformats timing node content and generates a timing message. 
    Replaces relative offset with absolute deadline from controlling thread
    and dispatches resulting message to it to priority queue 
    Bus access of priority queue is a single cycle and cannot be interrupted.
  * @param node Pointer to current node of type tmsg
  * @param thrData Pointer to the associated thread's metadata
  * @return Ptr to successor node  
*/
uint32_t* tmsg(uint32_t* node, uint32_t* thrData);

/// Processes a block node. Updates thread time sum and executes an available action from the command queues.
/** Block node is evaluated, thread time sum is increased by block period. 
  * The next available action in the command queues is executed and its quantity count decreased. 
  * If it reached zero, the action is popped from the queue. 
  * If action is a Flow, its destination node is returned to be processed next, otherwise the block's default successor node is.
  * @param node Pointer to current node of basetype block
  * @param thrData Pointer to the associated thread's metadata
  * @return Ptr to successor node (block's default successor or one from its alternative destinations list)
  */  
uint32_t* block(uint32_t* node, uint32_t* thrData);

/// Calls block node handler with fixed period (default)
/** Default block node processing, see block()
  * @param node Pointer to current node of type blockfixed
  * @param thrData Pointer to the associated thread's metadata
  * @return Ptr to successor node (block's default successor or one from its alternative destinations list)
  */ 
uint32_t* blockFixed(uint32_t* node, uint32_t* thrData);

/// Calls block node handler with a self aligning period
/** Block node period is dynamically elongated so the block ends aligned to a global time grid (currently 10µs starting at time 0)
  * @param node Pointer to current node of type blockaligned
  * @param thrData Pointer to the associated thread's metadata
  * @return Ptr to successor node (block's default successor or one from its alternative destinations list)
  */ 
uint32_t* blockAlign(uint32_t* node, uint32_t* thrData);

/** @name Default node, deadline and action handlers
 *  Used to handle invalid data, ie. nodes and actions of bad/unknown types 
 */
//@{ 
/// Handler for null node (idle), returns a null node
/** The returned next node to process is a null pointer, which will cause MAX_INT as deadline (see deadlineNull())
  * @param node Pointer to current node
  * @param thrData Pointer to the associated thread's metadata
  * @return null
  */ 
uint32_t* nodeNull (uint32_t* node, uint32_t* thrData);

/// Returns the deadline for a null node (idle)
/** The returned deadline will be MAX_INT, so the thread is not called again
  * @param node Pointer to the node whose deadline is to be determined (null)
  * @param thrData Pointer to the associated thread's metadata
  * @return 64b TAI timestamp with new deadline (MAX_INT)
*/
uint64_t  deadlineNull (uint32_t* node, uint32_t* thrData);

/// Dummy node function, used to catch bad node types
/** Reports bad/unknown node type to error register and calls the handler for a null node  
  * @param node Pointer to current node
  * @param thrData Pointer to the associated thread's metadata
  * @return null
  */ 
uint32_t* dummyNodeFunc (uint32_t* node, uint32_t* thrData);

/// Dummy node function, used to catch bad node types
/** Reports bad/unknown node type to error register and calls the handler for a null node  
  * @param node Pointer to current node
  * @param thrData Pointer to the associated thread's metadata
  * @return null
  */ 

uint32_t* dynamicNodeFunc (uint32_t* node, uint32_t* thrData);

/// Dummy deadline function, used to catch bad node types
/** Reports bad/unknown node type to error register and calls the handler for a null node deadline
  * @param node Pointer to the block basetype node whose deadline is to be determined
  * @param thrData Pointer to the associated thread's metadata
  * @return 64b TAI timestamp with new deadline (MAX_INT)
*/


uint64_t  dummyDeadlineFunc (uint32_t* node, uint32_t* thrData);

/// Dummy action function, used to catch bad action types
/** Reports bad/unknown command action type to error register and returns a nulll node as successor
  * @param node Pointer to current block
  * @param cmd Pointer to the command to be executed within current block's queues
  * @param thrData Pointer to the associated thread's metadata
*/
uint32_t* dummyActionFunc (uint32_t* node, uint32_t* cmd, uint32_t* thrData);
//@}

/** @name Heap sort routines
 *  Heap replace and full heap sort for EDF scheduler. Allows to find thread with earliest deadline 
 */
//@{ 
/// Sorts the whole heap of the EDF thread scheduler
/** EDF scheduler's heap is completely sorted by due deadline. 
    This is only necessary if threads were aborted or new threads were started because this can mean more than one changed element. 
    Otherwise a replace of the top element (earliest deadline) is sufficient. Stopped threads obtain MAX_INT 
    as new deadline and are moved to the bottom of the heap normally by a single heapReplace() */
void heapify();

/// Main routine of the EDF thread scheduler. Replaces the top element (earliest deadline) of the heap and sorts the new deadline.
/** Main routine of the EDF thread scheduler. Removes the top element (earliest deadline) from the heap and replaces it with its returned deadline.
    The new deadline is sorted into its proper place in the heap. Deactivated/Idle threads obtain MAX_INT 
    as new deadline and end up at the bottom of the heap.
  * @param src Index of heap element to be removed (replaced by follow ups). 0 is top of heap (earliest deadline).
*/
void heapReplace(uint32_t src);
//@}


#endif