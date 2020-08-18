#include <stdio.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
	int fd = open("firmware.bin", O_RDONLY);
	printf("DEPTH = 32768;\n"
			"WIDTH = 32;\n"
			"ADDRESS_RADIX = HEX;\n"
			"DATA_RADIX = HEX;\n"
			"CONTENT\n"
			"BEGIN\n");

	for (int i = 0; i < 32768; ++i) {
		uint32_t x, word;
		int result = read(fd, &x, 4);
		if (result != 4) {
			word = 0;
		} else {
			word  = (x>>24) & 0x000000ff;
			word |= (x<<24) & 0xff000000;
			word |= (x>>8)  & 0x0000ff00;
			word |= (x<<8)  & 0x00ff0000;
		}
		printf("%x : %08lx;\n", i, word);
	}
	printf("END;\n");


}