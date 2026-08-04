/********************************************************************************************
 *  dm_hal_mock.c
 *
 *  Host (mock) implementation of the DM firmware HAL (see dm_hal.h), used only by the
 *  host Unity test build (test/unit). Must NOT include any vendor SDK headers (aux.h,
 *  irq.h, mini_sdb.h, ebm.h, uart.h) - some of those contain LM32-only inline CSR
 *  assembly that cannot compile for a host/x86 target at all. Must also NOT include
 *  modules/datamaster-firmware/lm32-includes, which is reference-only.
 *
 *  All periphery state is a plain, HAL-owned fake register bank / fake shared-memory
 *  buffer - dmInit()/scheduler code under test operate on this instead of real
 *  dual-port RAM or SDB-discovered hardware registers.
 ********************************************************************************************/
#include <stdio.h>
#include <string.h>

#include "dm_hal.h"

/// Fake shared-memory buffer backing halGetSharedMemBase().
/** Sized generously to accommodate thread tables/heap/diagnostics; the real size
 *  (_SHCTL_END_) is generated per-project by the firmware build and not available on
 *  host, so a fixed, sufficiently large buffer is used instead. */
#define HAL_MOCK_SHARED_MEM_SIZE (4 * 1024)
extern uint32_t *const _startshared[];
uint32_t *sharedMem = (uint32_t *)_startshared;

extern uint8_t cpuId;

static uint64_t fakeSysTime = 0;
static uint8_t fakeWrTimeValid = 1;

static HalTimingMsg lastEbmMsg;
static uint8_t ebmMsgSent = 0;
static HalTimingMsg lastPrioQueueMsg;
static uint8_t prioQueueMsgSent = 0;
static uint64_t fakePrioQueueMsgCount = 0;
static uint32_t fakeEbmSrcIp = 0x0a000001; ///< fake WR-assigned source IP (10.0.0.1)

static uint32_t atomicNestCount = 0;

void halInitPeriphery(void)
{

  cpuId = halInitCpuId();

  memset(sharedMem, 0, HAL_MOCK_SHARED_MEM_SIZE);

  fakeSysTime = 1000;
  fakeWrTimeValid = 1;
  ebmMsgSent = 0;
  prioQueueMsgSent = 0;
  atomicNestCount = 0;
  fakePrioQueueMsgCount = 0;
}

uint64_t halGetSysTime(void)
{
  return fakeSysTime;
}

void halSetSysTime(uint64_t time)
{
  fakeSysTime = time;
}

void halIncSysTime(uint64_t time)
{
  fakeSysTime += time;
}

void halDecSysTime(uint64_t time)
{
  fakeSysTime -= time;
}

uint8_t halWrTimeValid(void)
{
  return fakeWrTimeValid;
}

void halEbmInit(void)
{
  // no-op in mock
  pp_printf("EBM init done\n");
}

void halEbmConfigMeta(uint32_t mtu, uint32_t hiBits, uint32_t ebOps)
{
  (void)mtu;
  (void)hiBits;
  (void)ebOps; // no-op in mock
}

void halEbmConfigIf(uint8_t conf, uint64_t mac, uint32_t ip, uint16_t port)
{
  (void)conf;
  (void)mac;
  (void)ip;
  (void)port; // no-op in mock
}

void halEbmWaitForIp(void)
{
  // returns immediately in mock, no busy-loop needed
  pp_printf("EBM ip wait done\n");
}

uint32_t halEbmGetSrcIp(void)
{
  return fakeEbmSrcIp;
}

void halEbmSend(const HalTimingMsg *msg)
{
  lastEbmMsg = *msg;
  ebmMsgSent = 1;
}

void halPrioQueueInit(void)
{
  // no-op in mock
  pp_printf("Prio init done\n");
}

void halPrioQueueSend(const HalTimingMsg *msg)
{
  lastPrioQueueMsg = *msg;
  prioQueueMsgSent = 1;
  ++fakePrioQueueMsgCount;
}

uint64_t halPrioQueueGetMsgCount(void)
{
  return fakePrioQueueMsgCount;
}

void halIrqSetup(uint32_t mask)
{
  return;
  /*
  isr_table_clr();
  irq_set_mask(mask);
  irq_disable();
  */
}

void halAtomicOn(void)
{
  ++atomicNestCount;
}

void halAtomicOff(void)
{
  --atomicNestCount;
}

int32_t halGetMsiBoxCpuSlot(uint8_t cpu, int32_t offset)
{
  (void)offset;
  return (cpu < 32) ? (int32_t)cpu : -1;
}

void halShowMsi(void)
{
  // no-op in mock
}

uint32_t *halGetSharedMemBase(void)
{
  return sharedMem;
}