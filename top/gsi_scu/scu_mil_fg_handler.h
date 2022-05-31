/*!
 * @file scu_mil_fg_handler.h
 * @brief Module for handling all MIL function generators and MIL DAQs
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author Ulrich Becker <u.becker@gsi.de>
 * @date 04.02.2020
 * Outsourced from scu_main.c
 */
#ifndef _SCU_MIL_FG_HANDLER_H
#define _SCU_MIL_FG_HANDLER_H
#if !defined( CONFIG_MIL_FG ) && !defined(__DOCFSM__)
  #error Macro CONFIG_MIL_FG has to be defined!
#endif
#ifndef __DOCFSM__
/* Headers will not need for FSM analysator "docfsm" */
  #include "scu_main.h"
  #include "scu_fg_macros.h"
#endif

//#define CONFIG_MIL_WAIT

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @ingroup ALIAS
 * @brief Alias name for member of control register in MIL-function generator
 * @see FG_CTRL_RG_T_BV::reset
 */

#define devDrq      reset

/*!
 * @ingroup ALIAS
 * @brief Alias name for member of control register in MIL-function generator
 * @see FG_CTRL_RG_T_BV::enable
 */
#define devStateIrq enable

extern FG_CHANNEL_T g_aFgChannels[MAX_FG_CHANNELS]; //!!

/*!
 * @brief Message queue for MIL-FGs filled by interrupt.
 */
extern SW_QUEUE_T g_queueMilFg;


#ifdef _CONFIG_VARIABLE_MIL_GAP_READING
 #ifndef CONFIG_READ_MIL_TIME_GAP
  #error CONFIG_READ_MIL_TIME_GAP has to be defined when _CONFIG_VARIABLE_MIL_GAP_READING is defined!
 #endif
 /*!
  * @brief MIL gap-reading time in milliseconds.
  * 
  * This variable is valid for all detected MIL function generator channels.
  * @note The value of zero means that the gap-reading is disabled.
  */
 extern unsigned int g_gapReadingTime;

 /*!
  * @brief Initializing value for global variable g_gapReadingTime
  *        in milliseconds.
  * @see g_gapReadingTime
  */
 #ifndef DEFAULT_GAP_READING_INTERVAL
  #define DEFAULT_GAP_READING_INTERVAL 0
 #endif
#endif /* ifdef _CONFIG_VARIABLE_MIL_GAP_READING */

#ifdef _CONFIG_DBG_MIL_TASK
void  dbgPrintMilTaskData( void );
#else
#define dbgPrintMilTaskData()
#endif


/*!
 * @brief Image of the MIL- function generators hardware registers.
 */
typedef struct HW_IMAGE
{
   /*!
    * @brief control register [r/w]
    */
   volatile FG_CTRL_RG_T cntrl_reg;

   /*!
    * @brief quadratic value 'a' [r/w]
    *
    * A 16 bit value that needs to be written once after each data
    * request interrupt.
    */
   volatile int16_t coeff_a_reg;

   /*!
    * @brief linear value 'b' [r/w]
    *
    * A 16 bit value that needs to be written once after each data
    * request interrupt.
    */
   volatile int16_t coeff_b_reg;

   /*!
    * @brief scale factor for value 'a' [r/w]
    *
    * Value must be between 0 and 64.
    */
   volatile uint16_t shift_reg;

   /*!
    * @brief C coefficient low value
    */
   volatile uint16_t coeff_c_low_reg;

   /*!
    * @brief C coefficient high value
    */
   volatile int16_t coeff_c_high_reg;

} FG_MIL_REGISTER_T;

STATIC_ASSERT( offsetof( FG_MIL_REGISTER_T, cntrl_reg )        == 0 * sizeof(uint16_t) );
STATIC_ASSERT( offsetof( FG_MIL_REGISTER_T, coeff_a_reg )      == 1 * sizeof(uint16_t) );
STATIC_ASSERT( offsetof( FG_MIL_REGISTER_T, coeff_b_reg )      == 2 * sizeof(uint16_t) );
STATIC_ASSERT( offsetof( FG_MIL_REGISTER_T, shift_reg )        == 3 * sizeof(uint16_t) );
STATIC_ASSERT( offsetof( FG_MIL_REGISTER_T, coeff_c_low_reg )  == 4 * sizeof(uint16_t) );
STATIC_ASSERT( offsetof( FG_MIL_REGISTER_T, coeff_c_high_reg ) == 5 * sizeof(uint16_t) );
STATIC_ASSERT( sizeof( FG_MIL_REGISTER_T ) == MIL_BLOCK_SIZE * sizeof(short) );

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Mecro declares a state of a Finite-State-Machine. \n
 *        Helper macro for documenting the FSM by the FSM-visualizer DOCFSM.
 * @see FG_STATE_T
 * @see https://github.com/UlrichBecker/DocFsm
 */
#define FSM_DECLARE_STATE( state, attr... ) state

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Declaration of the states of the task- FSM
 * @see https://github.com/UlrichBecker/DocFsm
 */
typedef enum
{
   FSM_DECLARE_STATE( ST_WAIT,            label='Wait for message', color=blue ),
#ifdef CONFIG_MIL_WAIT
   FSM_DECLARE_STATE( ST_PREPARE,         label='Request MIL-IRQ-flags\nclear old IRQ-flags', color=cyan ),
#endif
   FSM_DECLARE_STATE( ST_FETCH_STATUS,    label='Read MIL-IRQ-flags', color=green ),
   FSM_DECLARE_STATE( ST_HANDLE_IRQS,     label='Send data to\nfunction generator\nif IRQ-flag set', color=red ),
  // FSM_DECLARE_STATE( ST_DATA_AQUISITION, label='Request MIL-DAQ data\nif IRQ-flag set', color=cyan ),
   FSM_DECLARE_STATE( ST_FETCH_DATA,      label='Read MIL-DAQ data\nif IRQ-flag set',color=green )
} FG_STATE_T;

/*! ---------------------------------------------------------------------------
 * @ingroup PATCH
 * @brief Patch macro which accomplishes the register access of a
 *        ADAC function generator macro.
 * @see __FG_ACCESS
 * @param p Pointer to the concerning function generator register set.
 * @param m Name of member variable.
 * @code
 * MIL_FG_ACCESS( foo, bar ) = value;
 * @endcode
 * corresponds to
 * @code
 * foo->bar = value;
 * @endcode
 */
#define MIL_FG_ACCESS( p, m ) __FG_ACCESS( FG_MIL_REGISTER_T, uint16_t, p, m )

/*! ---------------------------------------------------------------------------
 * @brief helper function which clears the state of MIL-bus after malfunction
 */
void fgMilClearHandlerState( const unsigned int socket );

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief
 */
typedef struct
{  /*!
    * @brief Saved irq state, becomes initialized in function milGetStatus()
    *        respectively scub_get_task_mil() or get_task_mil()
    * @see milGetStatus
    * @see scub_get_task_mil
    * @see get_task_mil
    */
   int16_t   irqFlags;

   /*!
    * @brief Setvalue from the tuple sent
    */
   int       setvalue;

   /*!
    * @brief Timestamp of DAQ sampling.
    */
   uint64_t  daqTimestamp;
} FG_CHANNEL_TASK_T;

/*! -------------------------------------------------------------------------
 * @brief Payload data type for the message queue g_queueMilFg.
 *        A instance of this type becomes initialized by each interrupt.
 */
typedef struct
{ /*!
   * @brief Slave number respectively slot number of the controlling SIO-card
   *        when value > 0. When 0 then the the MIL extention is concerned.
   */
   unsigned int  slot;

   /*!
    * @brief Moment of interrupt which fills the queue of this object.
    */
   uint64_t      time;
} MIL_QEUE_T;

/*! --------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Task data type for MIL-FGs and MIL-DAQs
 * @see TASK_T
 * @see milDeviceHandler
 */
typedef struct
{  /*!
    * @brief Current FSM state
    */
   FG_STATE_T        state;

   /*!
    * @brief Contains the last message generated by interrupt.
    */
   MIL_QEUE_T        lastMessage;

   /*!
    * @brief Continuation of loop index for channel.
    */
   unsigned int      lastChannel;

   /*!
    * @brief timeout counter
    */
   unsigned int      timeoutCounter;
#ifdef CONFIG_MIL_WAIT
   /*!
    * @brief Waiting time after interrupt.
    */
   uint64_t          waitingTime;
#endif
#ifdef CONFIG_USE_INTERRUPT_TIMESTAMP
   /*!
    * @brief Duration in nanoseconds since the last interrupt.
    * 
    * For measurement and debug purposes only. 
    */
   uint64_t          irqDurationTime;
#endif

   /*!
    * @see FG_CHANNEL_TASK_T
    */
   FG_CHANNEL_TASK_T aFgChannels[ARRAY_SIZE(g_aFgChannels)];
} MIL_TASK_DATA_T;

extern MIL_TASK_DATA_T g_aMilTaskData[5];

/*! ---------------------------------------------------------------------------
 * @brief Scanns the SCU- bus for SIO-slaves with commected MIL- function
 *        generators, and put all found FGs in the function- generator list. 
 * @param scub_adr Baseaddress of SCU- bus.
 * @param pFgList Pointer to function generator list.
 */
void scanScuBusFgsViaMil( volatile uint16_t *scub_adr, FG_MACRO_T* pFgList );

/*! ---------------------------------------------------------------------------
 * @brief Scans the MIL extension (MIL-PIGGY) for function generators 
 *        and put all found FGs in the function- generator list. 
 */
void scanExtMilFgs( volatile unsigned int *mil_addr,
                    FG_MACRO_T* pFgList, uint64_t *ext_id );

/*! ---------------------------------------------------------------------------
 * @brief Returns the number of all found MIL- function generators after
 *        call of scanScuBusFgsViaMil and/or scanExtMilFgs.
 */
unsigned int milGetNumberOfFg( void );

/*! ---------------------------------------------------------------------------
 * @brief Helper function of fgEnableChannel handles the handler state
 *        of MIL devices.
 * @see fgEnableChannel
 * @param pScuBus Base address of SCU-bus.
 * @param pMilBus Base address of MIL-bus.
 * @param socket Socket number containing location information and FG-typ
 * @retval true leave the function fgEnableChannel
 * @retval false continue the function fgEnableChannel
 */
bool milHandleClearHandlerState( const void* pScuBus, const void* pMilBus,
                                 const unsigned int socket );

/*! ---------------------------------------------------------------------------
 * @brief Prepares the selected MIL- function generator.
 *
 * 1) Enabling the belonging interrupt \n
 * 2) Starts both belonging DAQ channels for feedback set- and actual- values. \n
 * 3) Sets the digital to analog converter in the function generator mode. \n
 *
 * @param pScuBus Base address of SCU-bus.
 * @param pMilBus Base address of MIL-bus.
 * @param socket Socket number containing location information and FG-typ
 * @param dev Device number of the concerning FG-device
 * @retval OKAY Action was successful
 */
void milFgPrepare( const void* pScuBus,
                   const void* pMilBus,
                   const unsigned int socket,
                   const unsigned int dev );

/*! ---------------------------------------------------------------------------
 * @brief Loads the selected ADDAC/ACU-function generator with the first
 *        polynomial data set and enable it.
 * @param pScuBus Base address of SCU-bus.
 * @param pMilBus Base address of MIL-bus.
 * @param pPset Pointer to the polynomial data set.
 * @param socket Socket number containing location and device type
 * @param dev Device number
 * @param channel Channel number of the concerned function generator.
 * @retval OKAY Action was successful.
 */
void milFgStart( const void* pScuBus,
                 const void* pMilBus,
                 const FG_PARAM_SET_T* pPset,
                 const unsigned int socket,
                 const unsigned int dev,
                 const unsigned int channel );

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief Disables the interrupts of a specific MIL- function generator.
 * @param pScuBus Base address of SCU-bus.
 * @param pMilBus Base address of MIL-bus.
 * @param socket Socket number containing location and device type
 * @param dev Device number
 */
void milFgDisableIrq( const void* pScuBus,
                      const void* pMilBus,
                      const unsigned int socket,
                      const unsigned int dev );

/*! ---------------------------------------------------------------------------
 */
int milFgDisable( const void* pScuBus,
                  const void* pMilBus,
                  unsigned int socket,
                  unsigned int dev );


#if defined( CONFIG_READ_MIL_TIME_GAP ) && !defined(__DOCFSM__)
/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Returns true, when the states of all MIL-FSMs are in the state
 *        ST_WAIT.
 */
bool isMilFsmInST_WAIT( void );

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Suspends the DAQ- gap reading. The gap reading becomes resumed once
 *        the concerning function generator has been sent its first data.
 */
void suspendGapReading( void );
#endif /* defined( CONFIG_READ_MIL_TIME_GAP ) */

/*! ---------------------------------------------------------------------------
 * @ingroup TASK
 * @ingroup MIL_FSM
 * @brief Pre-initializing of all MIL- task data.
 * 
 * @note This is necessary, especially after a LM32-reset because this
 *       objects are not in the .bss memory section.
 */
void milInitTasks( void );

/*! ---------------------------------------------------------------------------
 * @ingroup TASK
 * @ingroup MIL_FSM
 * @brief Non blocking execution of all MIL-threads.
 */
void milExecuteTasks( void );

#ifdef __cplusplus
}
#endif
#endif /* _SCU_MIL_FG_HANDLER_H */
/*================================== EOF ====================================*/
