#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <output_file>\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(argv[1], "wb");
    if (!fp) {
        perror("File open error");
        return 1;
    }

    unsigned int value;
    for (value = 0x04004000; value <= 0x040043FC; value += 4) {
        unsigned char bytes[4];
        bytes[0] = (value >> 24) & 0xFF;  // Most significant byte
        bytes[1] = (value >> 16) & 0xFF;
        bytes[2] = (value >> 8) & 0xFF;
        bytes[3] = value & 0xFF;          // Least significant byte

        fwrite(bytes, 1, 4, fp);
    }

    fclose(fp);
    printf("Binary file '%s' created.\n", argv[1]);
    return 0;
}
