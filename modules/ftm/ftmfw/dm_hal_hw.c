/********************************************************************************************
 *  dm_hal_hw.c
 *
 *  Real (hardware) implementation of the DM firmware HAL (see dm_hal.h).
 *
 *  Each function here is a thin, behaviour-preserving wrapper around the existing vendor
 *  SDK calls / register accesses already used by dm.c/main.c. Vendor headers are resolved
 *  exclusively via the existing SDK include paths configured in CMakeLists.txt
 *  (pkg_check_modules: lm32-include, lm32-wrpc-sw, timing-registers, i.e. /opt/sdk) -
 *  NEVER via modules/datamaster-firmware/lm32-includes, which is a read-only reference
 *  copy used only to design this HAL and must not be included by any implementation file.
 *
 *  This file is only compiled into the firmware (lm32) target.
 ********************************************************************************************/
#include "mini_sdb.h"
#include "irq.h"
#include "ebm.h"
#include "aux.h"
#include "uart.h"
#include "dbg.h"
#include "ftm_common.h"
#include "prio_regs.h"
#include "dm.h"
#include "dm_hal.h"

/// Shared RAM region base, provided by the linker script (ram.ld.S). Owned by the HAL;
/// dm.c/main.c must go through halGetSharedMemBase() instead of referencing this directly.
extern uint32_t* const _startshared[];

void halInitPeriphery(void)
{
  discoverPeriphery();
  cpuId = getCpuIdx();
}

uint64_t halGetSysTime(void)
{
  return getSysTime();
}

void halSetSysTime(uint64_t time)
{
  (void)time; // real WR system time cannot be set from firmware; test-only hook, no-op on real HAL
}

void halIncSysTime(uint64_t time)
{
  (void)time; // real WR system time cannot be incremented from firmware; test-only hook, no-op on real HAL
}

void halDecSysTime(uint64_t time)
{
  (void)time; // real WR system time cannot be decremented from firmware; test-only hook, no-op on real HAL
}

uint8_t halWrTimeValid(void)
{
  const uint32_t STATE_REG     = 0x1C;
  const uint32_t PPS_VALID_MSK = (1 << 2);
  const uint32_t TS_VALID_MSK  = (1 << 3);
  const uint32_t STATE_MSK     = PPS_VALID_MSK | TS_VALID_MSK;

  return ((pPps[STATE_REG >> 2] & STATE_MSK) != 0);
}

void halUartInitHw(void)
{
  uart_init_hw();
}

int halConsoleWrite(const char* text)
{
  return uart_write_string(text);
}

void halEbmInit(void)
{
  ebm_init();
}

void halEbmConfigMeta(uint32_t mtu, uint32_t hiBits, uint32_t ebOps)
{
  ebm_config_meta(mtu, hiBits, ebOps);
}

void halEbmConfigIf(uint8_t conf, uint64_t mac, uint32_t ip, uint16_t port)
{
  ebm_config_if((target_t)conf, mac, ip, port);
}

void halEbmWaitForIp(void)
{
  int j;
  while (*(pEbCfg + (EBC_SRC_IP >> 2)) == EBC_DEFAULT_IP) {
    for (j = 0; j < (125000000 / 2); ++j) { asm("nop"); }
  }
}

uint32_t halEbmGetSrcIp(void)
{
  return *(pEbCfg + (EBC_SRC_IP >> 2));
}

void halEbmSend(const HalTimingMsg* msg)
{
  ebm_hi(ECA_GLOBAL_ADR);
  ebm_op(ECA_GLOBAL_ADR, msg->idHi, EBM_WRITE);
  ebm_op(ECA_GLOBAL_ADR, msg->idLo, EBM_WRITE);
  ebm_op(ECA_GLOBAL_ADR, msg->parHi, EBM_WRITE);
  ebm_op(ECA_GLOBAL_ADR, msg->parLo, EBM_WRITE);
  ebm_op(ECA_GLOBAL_ADR, msg->res, EBM_WRITE);
  ebm_op(ECA_GLOBAL_ADR, msg->tef, EBM_WRITE);
  ebm_op(ECA_GLOBAL_ADR, msg->tsHi, EBM_WRITE);
  ebm_op(ECA_GLOBAL_ADR, msg->tsLo, EBM_WRITE);
  ebm_flush();
}

void halPrioQueueInit(void)
{
  pFpqCtrl[PRIO_RESET_OWR >> 2]      = 1;
  pFpqCtrl[PRIO_MODE_CLR >> 2]       = 0xffffffff;
  pFpqCtrl[PRIO_ECA_ADR_RW >> 2]     = ECA_GLOBAL_ADR;
  pFpqCtrl[PRIO_EBM_ADR_RW >> 2]     = ((uint32_t)pEbm & ~0x80000000);
  pFpqCtrl[PRIO_TX_MAX_MSGS_RW >> 2] = 40;
  pFpqCtrl[PRIO_TX_MAX_WAIT_RW >> 2] = loW((uint64_t)(50000));
  pFpqCtrl[PRIO_MODE_SET >> 2]       = PRIO_BIT_ENABLE     |
                                        PRIO_BIT_MSG_LIMIT  |
                                        PRIO_BIT_TIME_LIMIT;
}

void halPrioQueueSend(const HalTimingMsg* msg)
{
  *(pFpqData + (PRIO_DAT_STD   >> 2)) = msg->idHi;
  *(pFpqData + (PRIO_DAT_STD   >> 2)) = msg->idLo;
  *(pFpqData + (PRIO_DAT_STD   >> 2)) = msg->parHi;
  *(pFpqData + (PRIO_DAT_STD   >> 2)) = msg->parLo;
  *(pFpqData + (PRIO_DAT_STD   >> 2)) = msg->res;
  *(pFpqData + (PRIO_DAT_STD   >> 2)) = msg->tef;
  *(pFpqData + (PRIO_DAT_TS_HI >> 2)) = msg->tsHi;
  *(pFpqData + (PRIO_DAT_TS_LO >> 2)) = msg->tsLo;
}

uint64_t halPrioQueueGetMsgCount(void)
{
  return *(uint64_t*)&pFpqCtrl[PRIO_CNT_OUT_ALL_GET_0 >> 2];
}

void halIrqSetup(uint32_t mask)
{
  isr_table_clr();
  irq_set_mask(mask);
  irq_disable();
}

void halAtomicOn(void)
{
  atomic_on();
}

void halAtomicOff(void)
{
  atomic_off();
}

int32_t halGetMsiBoxCpuSlot(uint8_t cpu, int32_t offset)
{
  return (int32_t)getMsiBoxCpuSlot(cpu, (uint32_t)offset);
}

void halShowMsi(void)
{
  pp_printf(" Msg:\t%08x\nAdr:\t%08x\nSel:\t%01x\n", global_msi.msg, global_msi.adr, global_msi.sel);
}

uint32_t* halGetSharedMemBase(void)
{
  return (uint32_t*)&_startshared;
}
