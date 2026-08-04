#include "mini_sdb.h"
#include "hw/memlayout.h"
#include "uart.h"

extern uint8_t cpuId;

uint8_t halInitCpuId()
{
    pCpuId = find_device_adr(GSI, CPU_INFO_ROM);
    return (uint8_t)*pCpuId & 0xff;
}

void halUartInitHw(void)
{
    // no-op in mock
    BASE_UART = (char *)find_device_adr(CERN, WR_UART);
    uart_init_hw();
}

void halOnlyMockDoesStuff(void)
{
    for (int j = 0; j < ((125000000 / 4) + (cpuId * 2500000)); ++j)
    {
        asm("nop");
    }
    if (cpuId == 0)
        pp_printf("#%02u: Alive %s \n", cpuId, DM_RELEASE, DM_VERSION);
}

int halConsoleWrite(const char *text)
{
    return uart_write_string(text);
}