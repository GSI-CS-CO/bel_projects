#include "mini_sdb.h"

void* memset(void* s, int c, int n) {
	for (int i = 0; i < n; ++i) {
		*(int*)s = c;
	}
}


volatile int x;
int main() {
	discoverPeriphery();

	for(;;) {
		for(int i = 0; i < 32; ++i) {
			int adr = (1<<i);
			x = *(int*)adr;
		}
	}
	return 0;
}