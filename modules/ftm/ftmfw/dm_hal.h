/********************************************************************************************
 *  dm_hal.h
 *
 *  Hardware Abstraction Layer (HAL) for DM firmware periphery access.
 *
 *  This header declares a hardware-agnostic interface for all external-resource /
 *  periphery touchpoints used by dm.c/main.c (WR/PPS timing core, Priority Queue,
 *  EBM/Etherbone, UART, SDB peripheral discovery, IRQ controller, MSI mailbox and the
 *  shared-RAM region). It has exactly two implementations:
 *
 *    - dm_hal_hw.c   (real implementation, firmware/lm32 build only, wraps the actual
 *                     vendor SDK calls resolved via the existing pkg_check_modules
 *                     include paths)
 *    - test/mocks/dm_hal_mock.c (mock implementation, host unit test build only)
 *
 *  IMPORTANT: this header must stay fully portable C (no vendor SDK headers, no
 *  target-specific inline asm) so it can be included unmodified by both dm.c
 *  (firmware build) and the host Unity test build.
 ********************************************************************************************/
#ifndef _DM_HAL_H_
#define _DM_HAL_H_

#include <stdint.h>

/** @name Periphery discovery / CPU identity
 */
//@{
/// Discovers periphery and determines this CPU's index.
/** Real impl: discoverPeriphery() + getCpuIdx(). Mock impl: allocates a fake register
 *  bank and a fake shared-memory buffer instead of walking a real SDB bus. */
void halInitPeriphery(void);
//@}

/** @name WR/PPS timing core
 */
//@{
uint64_t halGetSysTime(void);          ///< Current WR system time (real: getSysTime(); mock: fake clock)
void     halSetSysTime(uint64_t time); ///< Test-only setter for the mock clock (no-op on real HAL)
void     halIncSysTime(uint64_t time); ///< Test-only increment for the mock clock (no-op on real HAL)
uint8_t  halWrTimeValid(void);         ///< Whether WR time is valid/synced (real: pPps state bits; mock: fake PPS flag)
//@}

/// Shared timing-message fields dispatched via the Priority Queue or EBM.
/** Mirrors the fields written to pFpqData / passed to ebm_hi/ebm_op in tmsg(). Using a
 *  named struct instead of positional uint32_t parameters avoids argument-order errors
 *  between callers and keeps the real/mock implementations from silently desyncing. */
typedef struct {
  uint32_t idHi;
  uint32_t idLo;
  uint32_t parHi;
  uint32_t parLo;
  uint32_t res;
  uint32_t tef;
  uint32_t tsHi;
  uint32_t tsLo;
} HalTimingMsg;

/** @name UART
 */
//@{
void halUartInitHw(void); ///< Initializes UART hardware (real: uart_init_hw(); mock: no-op)
//@}

/** @name Console output
 */
//@{
/// Writes an already formatted, NUL-terminated console string.
/** Real impl: uart_write_string(). Mock impl: fputs(..., stdout). */
int halConsoleWrite(const char* text);
//@}

/** @name Etherbone Master (EBM)
 */
//@{
void halEbmInit(void);                                                        ///< real: ebm_init(); mock: no-op/reset fake state
void halEbmConfigMeta(uint32_t mtu, uint32_t hiBits, uint32_t ebOps);          ///< real: ebm_config_meta(); mock: records args
void halEbmConfigIf(uint8_t conf, uint64_t mac, uint32_t ip, uint16_t port);   ///< real: ebm_config_if(); mock: records args
void halEbmWaitForIp(void);                                                   ///< real: polls pEbCfg/EBC_DEFAULT_IP; mock: returns immediately
uint32_t halEbmGetSrcIp(void);                                                ///< real: reads pEbCfg/EBC_SRC_IP (valid once halEbmWaitForIp() returns); mock: fake IP
void halEbmSend(const HalTimingMsg* msg);                                     ///< real: ebm_hi/ebm_op/ebm_flush sequence; mock: records dispatched msg

/// Portable equivalents of ebm.h's target_t SOURCE/DESTINATION and the EBM_NOREPLY flag,
/// so callers (main.c) don't need to include the vendor ebm.h just for these constants.
#define HAL_EBM_SOURCE      0
#define HAL_EBM_DESTINATION 1
#define HAL_EBM_NOREPLY      (1u << 28)
//@}

/** @name Priority Queue (PQ)
 */
//@{
void halPrioQueueInit(void);                ///< real: pFpqCtrl[...] config writes; mock: no-op/reset fake state
void halPrioQueueSend(const HalTimingMsg* msg); ///< real: pFpqData writes; mock: records dispatched msg
uint64_t halPrioQueueGetMsgCount(void);     ///< real: reads pFpqCtrl/PRIO_CNT_OUT_ALL_GET_0; mock: fake counter
//@}

/** @name IRQ / atomic sections
 */
//@{
void halIrqSetup(uint32_t mask); ///< real: isr_table_clr()+irq_set_mask()+irq_disable(); mock: no-op
void halAtomicOn(void);          ///< real: atomic_on(); mock: no-op or reentrancy-counting fake
void halAtomicOff(void);         ///< real: atomic_off(); mock: no-op or reentrancy-counting fake
//@}

/** @name MSI mailbox
 */
//@{
int32_t halGetMsiBoxCpuSlot(uint8_t cpu, int32_t offset); ///< real: getMsiBoxCpuSlot(); mock: fake slot allocator
void    halShowMsi(void);                                 ///< real: prints global_msi; mock: no-op
//@}

/** @name Shared RAM region
 */
//@{
uint32_t* halGetSharedMemBase(void); ///< Base of the shared RAM region (replaces direct use of _startshared)
//@}

#endif /* _DM_HAL_H_ */
