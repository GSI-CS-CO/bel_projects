/********************************************************************************************
 *  dm_console.c
 *
 *  Console-output wrapper for the DM firmware.
 *
 *  pp_printf() formats text and calls puts().  Interposing that call here preserves the
 *  shared pp_printf source unchanged while allowing the selected HAL implementation to
 *  route output to either the LM32 UART or host standard output.
 ********************************************************************************************/
#include "dm_hal.h"

int puts(const char* text)
{
  return halConsoleWrite(text);
}