#include <stack.h>
#include <assert.h>
#include <pp-printf.h>
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
  
  // lazily, initialize at first invocation (no init function is there) 
  static int  inited;
  char help[64];
  int i;
  
  if (!inited) {
    inited++;
    _endram = ENDRAM_MAGIC;
  }
  if (_endram != ENDRAM_MAGIC) {
    // avoid trailing '/0'
    pp_sprintf(help, "Stack overflow! (0x%x)", (unsigned int)_endram);
    for (i=0; i<strlen(help); i++) ((char *)fwid)[i+STACKSTATUS_OFFSET] = help[i];
  } // if _endram
} // check_stack_fwid

