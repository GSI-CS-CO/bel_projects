#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <output_file>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];

    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("File open error");
        return 1;
    }

    unsigned char word[4];
    //csr see dma engine doc, transfer size 16
    word[0] = 0b00000000; // bits 7 to 0
    word[1] = 0b00011100; // bits 15 to 8
    word[2] = 0b00000000; // bits 23 to 16
    word[3] = 0b00010000; // bits 31 to 24
    fwrite(&word, 4, 1, fp);

    //adr0 0x04004000
    word[0] = 0b00000100; // bits 7 to 0
    word[1] = 0b00000000; // bits 15 to 8
    word[2] = 0b01000000; // bits 23 to 16
    word[3] = 0b00000000; // bits 31 to 24
    fwrite(&word, 4, 1, fp);

    //adr1 0x04004400
    word[0] = 0b00000100; // bits 7 to 0
    word[1] = 0b00000000; // bits 15 to 8
    word[2] = 0b01000100; // bits 23 to 16
    word[3] = 0b00000000; // bits 31 to 24
    fwrite(&word, 4, 1, fp);

    //desc_next
    word[0] = 0b00000000; // bits 7 to 0
    word[1] = 0b00000000; // bits 15 to 8
    word[2] = 0b00000000; // bits 23 to 16
    word[3] = 0b00000000; // bits 31 to 24
    fwrite(&word, 4, 1, fp);

    fclose(fp);
    printf("Descriptor in '%s' created.\n", filename);
    return 0;
}
