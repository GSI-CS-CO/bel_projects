void* memset(void* dest, int value, unsigned len) {
	void* result = dest;
	while(len > 0) {
		*(int*)dest = value;
		len -= 4;
	}
	return result;
}

int main() {
	for (;;) {
		volatile int *ptr = (volatile int*)(0x20250);
		for (int i = 0; i < 0x20; ++i) {
			//int x = ptr[i];
			// ptr[i] = i;
		}

	}
}