#include <stack.h>
#include <assert.h>
#include <pp-printf.h>
#include <aux.h>
#include <string.h>

void check_stack(void)
{
	/* lazily, initialize at first invocation (no init function is there */
	static int inited;
	if (!inited) {
		inited++;
		_endram = ENDRAM_MAGIC;
	}
	assert(_endram == ENDRAM_MAGIC, "Stack overflow! (%x)\n", (unsigned int)_endram);
}


// routine writes info to a dedicated offset within the lm32 firmware ID
void check_stack_fwid(uint32_t *fwid)
{
#define STACKSTATUS_OFFSET 24
  
  static int  inited;
  char        help[64];
  int         i,j;

  // lazily, initialize at first invocation (no init function is there) 
  if (!inited) {
    inited++;
    _endram = ENDRAM_MAGIC;
    fwid[(STACKSTATUS_OFFSET >> 2)] = 0x6f6b6f6b;
  }
  if (_endram != ENDRAM_MAGIC) {
    // print error message and value to firmware ID
    // avoid trailing '/0'
    pp_sprintf(help, "Stack overflow! (0x%x", (unsigned int)_endram);
    for (i=0; i<strlen(help); i++) ((char *)fwid)[i+STACKSTATUS_OFFSET] = help[i];

    // print actual WR time to firmware ID too
    j = i;
    pp_sprintf(help, ", 0x%016llx)", getSysTime());
    for (i=0; i<strlen(help); i++) ((char *)fwid)[i+j+STACKSTATUS_OFFSET] = help[i];    

    assert(0, "Stack overflow! (%x)\n", (unsigned int)_endram);
  } // if _endram
} // check_stack_fwid

