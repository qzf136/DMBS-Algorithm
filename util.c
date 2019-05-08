#include <stdio.h>
#include <stdlib.h>
#include "extmem.c"

# define blkSize  64
# define bufSize  520

# define TUPLE_SIZE  8
# define BLOCK_TUPLE_NUM  7

# define R_BLOCK_NUM  16
# define S_BLOCK_NUM  32

# define R_BEGIN_DISK  100
# define S_BEGIN_DISK  200

void generateRS(Buffer *buf) {
    int i, j;
    int diskID = R_BEGIN_DISK;
    int nextDisk, lastDisk;
    unsigned char *blk_char = NULL;
    unsigned int *blk_int = NULL;
    for (i = 0; i < R_BLOCK_NUM; i++) {
        blk_char = getNewBlockInBuffer(buf);
        blk_int = (unsigned int*)blk_char;
        for (j = 0; j < BLOCK_TUPLE_NUM; j++) {
            *(blk_int+2*j) = rand()%(40)+1;
            *(blk_int+2*j+1) = rand()%1000+1;
        }
        nextDisk = diskID+1;
        lastDisk = diskID-1;
        if (i == R_BLOCK_NUM-1) nextDisk = 0;
        if (i == 0)  lastDisk = 0;
        *(blk_int+2*j) = nextDisk;
        *(blk_int+2*j+1) = lastDisk;
        writeBlockToDisk(blk_char, diskID, buf);
        freeBlockInBuffer(blk_char, buf);
        diskID++;
        nextDisk++;
    }
    diskID = S_BEGIN_DISK;
    for (i = 0; i < S_BLOCK_NUM; i++) {
        blk_char = getNewBlockInBuffer(buf);
        blk_int = (unsigned int*)blk_char;
        for (j = 0; j < BLOCK_TUPLE_NUM; j++) {
            *(blk_int+2*j) = rand()%(41)+20;
            *(blk_int+2*j+1) = rand()%1000+1;
        }
        nextDisk = diskID+1;
        lastDisk = diskID-1;
        if (i == S_BLOCK_NUM-1) nextDisk = 0;
        if (i == 0)  lastDisk = 0;
        *(blk_int+2*j) = nextDisk;
        *(blk_int+2*j+1) = lastDisk;
        writeBlockToDisk(blk_char, diskID, buf);
        freeBlockInBuffer(blk_char, buf);
        diskID++;
        nextDisk++;
    }
}

void printBlock(int disk, Buffer *buf) {
    int i = 0;
    unsigned char *blk_char;
    unsigned int *blk_int;
    blk_char = readBlockFromDisk(disk, buf);
    blk_int = (unsigned int *)blk_char;
    while (i < BLOCK_TUPLE_NUM) {
        printf("%d\t%d\n", blk_int[2*i], blk_int[2*i+1]);
        i++;
    }
    printf("next: %d\t last:%d\n", blk_int[2*i], blk_int[2*i+1]);
    freeBlockInBuffer(blk_char, buf);
}

void writeRS(int R_beign, int S_begin, Buffer *buf) {
    FILE *fp;
    unsigned char *blk_char;
    unsigned int *blk_int;
    int i, j;
    fp = fopen("R.txt", "w");
    for (i = 0; i < R_BLOCK_NUM; i++) {
        blk_char = readBlockFromDisk(R_beign+i, buf);
        blk_int = (unsigned int *)blk_char;
        fprintf(fp, "%d\n", R_beign+i);
        for (j = 0; j < BLOCK_TUPLE_NUM; j++) {
            fprintf(fp, "%d\t%d\n", blk_int[2*j], blk_int[2*j+1]);
        }
        fprintf(fp, "next: %d\t last: %d\n\n", blk_int[2*j], blk_int[2*j+1]);
        freeBlockInBuffer(blk_char, buf);
    }
    fclose(fp);
    
    fp = fopen("S.txt", "w");
    for (i = 0; i < S_BLOCK_NUM; i++) {
        blk_char = readBlockFromDisk(S_begin+i, buf);
        blk_int = (unsigned int *)blk_char;
        fprintf(fp, "%d\n", S_begin+i);
        for (j = 0; j < BLOCK_TUPLE_NUM; j++) {
            fprintf(fp, "%d\t%d\n", blk_int[2*j], blk_int[2*j+1]);
        }
        fprintf(fp, "next: %d\t last: %d\n\n", blk_int[2*j], blk_int[2*j+1]);
        freeBlockInBuffer(blk_char, buf);
    }
    fclose(fp);
}

