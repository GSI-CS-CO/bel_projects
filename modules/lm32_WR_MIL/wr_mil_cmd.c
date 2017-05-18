#include <wr_mil_cmd.h>
#include "mini_sdb.h"
#include "mprintf.h"



extern uint32_t _startshared[]; // provided in linker script "ram.ld"
                                // This has to be an array, not a pointer!
#define SHARED __attribute__((section(".shared")))
//uint64_t SHARED dummy = 0;

volatile MilCmdRegs *MilCmd_init(uint32_t *device_addr)
{
  if (!device_addr)
  {
    volatile MilCmdRegs *cmd = (volatile MilCmdRegs*)_startshared;
    cmd->cmd = 0;
    return cmd;
  }
  return (volatile MilCmdRegs*)device_addr;
}

// check if the command (cmd) register is != 0, 
//  do stuff according to its value, and set it 
//  back to 0
void MilCmd_poll(volatile MilCmdRegs *cmd)
{
  if (cmd->cmd)
  {
    switch(cmd->cmd)
    {
      case 0x00000001:
        mprintf("stop MCU\n");
        while(1);
      default:
        mprintf("found command %08x\n", cmd->cmd);
        break;
    }
    cmd->cmd = 0;
  }   
}
