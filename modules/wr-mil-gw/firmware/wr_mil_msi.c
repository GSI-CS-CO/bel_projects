#include "wr_mil_msi.h"

#include "mini_sdb.h"


void send_MSI(uint32_t slot, uint32_t msg)
{
    volatile uint32_t *msi = pCpuMsiBox + slot*2;
    if (slot != UINT32_C(0xffffffff)) { // only send MSI if slot is configured to valid slot
    	*msi = msg; // trigger MSI by writing into this register of the mailbox
    }
}
