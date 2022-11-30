/*
 * This work is part of the White Rabbit project
 *
 * Copyright (C) 2013 GSI (www.gsi.de)
 * Author: Wesley W. Terpstra <w.terpstra@gsi.de>
 *
 * Released according to the GNU GPL, version 2 or any later version.
 */
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	unsigned char x[4];
	int i, n;
	FILE* f;
	
	
	if (argc < 3) return 1;
	if (!(f = fopen(argv[1], "rb"))) return 1;
	
	n = atoi(argv[2])/4;
	
	printf("DEPTH = %d;\n", n);
	printf("WIDTH = 32;\n");
	printf("ADDRESS_RADIX = HEX;\n");
	printf("DATA_RADIX = HEX;\n");
	printf("CONTENT\n");
	printf("BEGIN\n");
	
	for (i = 0; !feof(f); ++i) {
		fread(x, 1, 4, f);
		printf("%x : %02X%02X%02X%02X;\n", i, x[0], x[1], x[2], x[3]);
		// printf("%x : E0000000;\n", i);
	}

	for (; i < n; ++i) {
		printf("%x : %02X%02X%02X%02X;\n", i, 0, 0, 0, 0);
	}

	printf("END;\n");
	fclose(f);
	return 0;
}
