#include <stack.h>
#include <assert.h>

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
