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
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern FG_CHANNEL_T g_aFgChannels[MAX_FG_CHANNELS]; //!!

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
  #define DEFAULT_GAP_READING_INTERVAL 10
 #endif
#endif /* ifdef _CONFIG_VARIABLE_MIL_GAP_READING */

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
   FSM_DECLARE_STATE( ST_PREPARE,         label='Request MIL-IRQ-flags\nclear old IRQ-flags', color=cyan ),
   FSM_DECLARE_STATE( ST_FETCH_STATUS,    label='Read MIL-IRQ-flags', color=green ),
   FSM_DECLARE_STATE( ST_HANDLE_IRQS,     label='Send data to\nfunction\nif IRQ-flag set', color=red ),
   FSM_DECLARE_STATE( ST_DATA_AQUISITION, label='Request MIL-DAQ data\nif IRQ-flag set', color=cyan ),
   FSM_DECLARE_STATE( ST_FETCH_DATA,      label='Read MIL-DAQ data\nif IRQ-flag set',color=green )
} FG_STATE_T;


/*! ---------------------------------------------------------------------------
 * @brief helper function which clears the state of a dev bus after malfunction
 */
void clear_handler_state( const unsigned int socket );

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief
 */
typedef struct
{
   int16_t   irq_data;      /*!<@brief saved irq state */
   int       setvalue;      /*!<@brief setvalue from the tuple sent */
   uint64_t  daq_timestamp; /*!<@brief timestamp of daq sampling */
} FG_CHANNEL_TASK_T;

/*! --------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Task data type for MIL-FGs and MIL-DAQs
 * @see TASK_T
 * @see milDeviceHandler
 */
typedef struct
{
   FG_STATE_T        state;            /*!<@brief current FSM state */
   unsigned int      slave_nr;         /*!<@brief slave nr of the controlling sio card
                                                  its the SCU-slot number when > 0,
                                                  in the case of zero the MIL-extention */
   unsigned int      lastChannel;      /*!<@brief loop index for channel */
   unsigned int      task_timeout_cnt; /*!<@brief timeout counter */
   uint64_t          timestamp1;       /*!<@brief timestamp */
#ifdef CONFIG_READ_MIL_TIME_GAP
   uint64_t          gapReadingTime; // Workaround!!! Move this in FG_CHANNEL_T resp. g_aFgChannels!!!
#endif
   FG_CHANNEL_TASK_T aFgChannels[ARRAY_SIZE(g_aFgChannels)]; /*!<@see FG_CHANNEL_TASK_T */
} MIL_TASK_DATA_T;

extern MIL_TASK_DATA_T g_aMilTaskData[5];

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
 * @brief can have multiple instances, one for each active sio card controlling
 * a dev bus persistent data, like the state or the sio slave_nr, is stored in
 * a global structure
 * @param pThis pointer to the current task object
 * @see schedule
 */
void dev_sio_handler( register TASK_T* pThis );

/*! ---------------------------------------------------------------------------
 * @ingroup TASK
 * @ingroup MIL_FSM
 * @brief has only one instance
 * persistent data is stored in a global structure
 * @param pThis pointer to the current task object
 * @see schedule
 */
void dev_bus_handler( register TASK_T* pThis );

#ifdef __cplusplus
}
#endif
#endif /* _SCU_MIL_FG_HANDLER_H */
/*================================== EOF ====================================*/
