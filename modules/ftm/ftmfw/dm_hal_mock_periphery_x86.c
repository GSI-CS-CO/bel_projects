#include "dm_hal.h"
#include <stdio.h>

uint8_t halInitCpuId()
{
    return 1; // always return 1 for mock
}

void halUartInitHw(void)
{
    // no-op in mock
    return;
}

int halConsoleWrite(const char *text)
{
    return fputs(text, stdout);
}

void halOnlyMockDoesStuff(void) { return; }