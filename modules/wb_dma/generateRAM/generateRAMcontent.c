#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <output_file> <size_in_bytes>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    long filesize = atol(argv[2]);
    if (filesize <= 0) {
        printf("Invalid file size.\n");
        return 1;
    }

    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("File open error");
        return 1;
    }

    srand((unsigned int)time(NULL));
    for (long i = 0; i < filesize; ++i) {
        unsigned char byte = rand() % 256;
        fwrite(&byte, 1, 1, fp);
    }

    fclose(fp);
    printf("Random binary file '%s' of %ld bytes created.\n", filename, filesize);
    return 0;
}
