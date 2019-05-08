#include <stdio.h>
#include <stdlib.h>
#include "util.c"

void linearSelect(int R_A, int S_C, Buffer *buf);
void mergeSortAllDisk(Buffer *buf);
void mergeSort(int begin1, int begin2, int diskNum, int output, Buffer *buf);
void swap(int *p1, int *p2);
void sortDisk(int disk, Buffer *buf);
void binarySearch(int R_A, int S_C, int *buf);

int main(int argc, char const *argv[])
{
    Buffer buf;
    initBuffer(bufSize, blkSize, &buf);
    generateRS(&buf);
    // writeRS(R_BEGIN_DISK, S_BEGIN_DISK, &buf);
    // linearSelect(40, 60, &buf);
    mergeSortAllDisk(&buf);
    // writeRS(R_BEGIN_DISK, S_BEGIN_DISK, &buf);
    return 0;
}

void linearSelect(int R_A, int S_C, Buffer *buf) {
    int i, j;
    int k = 0;
    int DISK = 0;
    unsigned char *blk_char = NULL;
    unsigned int *blk_int = NULL;
    unsigned char *result_blk_char = NULL;
    unsigned int *result_blk_int = NULL;
    result_blk_char = getNewBlockInBuffer(buf);
    result_blk_int = (unsigned int *)result_blk_char;
    for (i=0; i <R_BLOCK_NUM; i++) {
        blk_char = readBlockFromDisk(R_BEGIN_DISK+i, buf);
        blk_int = (unsigned int*)blk_char;
        for (j = 0; j < BLOCK_TUPLE_NUM; j++) {
            if (blk_int[2*j] == R_A) {
                result_blk_int[2*k] = blk_int[2*j];
                result_blk_int[2*k+1] = blk_int[2*j+1];
                k++;
            }
            if (k == BLOCK_TUPLE_NUM) {
                result_blk_int[2*k] = DISK+1;
                result_blk_int[2*k+1] = -1;
                writeBlockToDisk(result_blk_char, DISK, buf);
                result_blk_char = getNewBlockInBuffer(buf);
                result_blk_int = (unsigned int *)result_blk_char;
                k = 0;
                DISK++;
            }
        }
        freeBlockInBuffer(blk_char, buf);
    }
    for (i=0; i <S_BLOCK_NUM; i++) {
        blk_char = readBlockFromDisk(S_BEGIN_DISK+i, buf);
        blk_int = (unsigned int*)blk_char;
        for (j = 0; j < BLOCK_TUPLE_NUM; j++) {
            if (blk_int[2*j] == S_C) {
                result_blk_int[2*k] = blk_int[2*j];
                result_blk_int[2*k+1] = blk_int[2*j+1];
                k++;
            }
            if (k == BLOCK_TUPLE_NUM) {
                result_blk_int[2*k] = DISK+1;
                result_blk_int[2*k+1] = -1;
                writeBlockToDisk(result_blk_char, DISK, buf);
                result_blk_char = getNewBlockInBuffer(buf);
                result_blk_int = (unsigned int *)result_blk_char;
                k = 0;
                DISK++;
            }
        }
        freeBlockInBuffer(blk_char, buf);
    }
    if (k != 0) {
        result_blk_int[2*k] = 0;
        result_blk_int[2*k+1] = -1;
        k++;
        while (k < BLOCK_TUPLE_NUM+1) {
            result_blk_int[2*k] = 0;
            result_blk_int[2*k+1] = 0;
            k++;
        }
        writeBlockToDisk(result_blk_char, DISK, buf);
    }
}

void mergeSort(int begin1, int begin2, int diskNum, int output, Buffer *buf) {
    unsigned char *blk_char1 = NULL, *blk_char2 = NULL;
    unsigned int *blk_int1 = NULL, *blk_int2 = NULL;
    unsigned char *blk_char_result = NULL;
    unsigned int *blk_int_result = NULL;
    int i = 0, j = 0, k = 0;   // 内存块中的下标
    int flag1 = 0, flag2 = 0;   // 两个磁盘指针是否读入数据
    int readDiskNum1 = 0, readDiskNum2 = 0;
    int disk1 = begin1, disk2 = begin2;
    int outputDisk = output;
    blk_char_result = getNewBlockInBuffer(buf);
    blk_int_result = (unsigned int*)blk_char_result;
    while (1) {
        if (flag1 == 0 && readDiskNum1 < diskNum) {
            blk_char1 = readBlockFromDisk(disk1, buf);
            blk_int1 = (unsigned int*)blk_char1;
            flag1 = 1;
            i = 0;
            readDiskNum1++;
        }
        if (flag2 == 0 && readDiskNum2 < diskNum) {
            blk_char2 = readBlockFromDisk(disk2, buf);
            blk_int2 = (unsigned int*)blk_char2;
            flag2 = 1;
            j = 0;
            readDiskNum2++;
        }
        while ((i != BLOCK_TUPLE_NUM) && (j != BLOCK_TUPLE_NUM)) {
            if (blk_int1[2*i] > blk_int2[2*j]) {
                blk_int_result[2*k] = blk_int2[2*j];
                blk_int_result[2*k+1] = blk_int2[2*j+1];
                k++;
                j++;
            } else {
                blk_int_result[2*k] = blk_int1[2*i];
                blk_int_result[2*k+1] = blk_int1[2*i+1];
                k++;
                i++;
            }
            if (k == BLOCK_TUPLE_NUM) {
                blk_int_result[2*k] = outputDisk+1;
                blk_int_result[2*k+1] = -1;
                writeBlockToDisk(blk_char_result, outputDisk, buf);
                blk_char_result = getNewBlockInBuffer(buf);
                blk_int_result = (unsigned int*)blk_char_result;
                outputDisk++;
                k = 0;
            }
            if (i == BLOCK_TUPLE_NUM) {
                flag1 = 0;
                disk1++;
                freeBlockInBuffer(blk_char1, buf);
            }
            if (j == BLOCK_TUPLE_NUM) {
                flag2 = 0;
                disk2++;
                freeBlockInBuffer(blk_char2, buf);
            }
        }
        if (readDiskNum1==diskNum && i == BLOCK_TUPLE_NUM) {
            if (j < BLOCK_TUPLE_NUM) {
                while (j < BLOCK_TUPLE_NUM) {
                    blk_int_result[2*k] = blk_int2[2*j];
                    blk_int_result[2*k+1] = blk_int2[2*j+1];
                    j++;
                    k++;
                }
                flag2 = 0;
                disk2++;
                freeBlockInBuffer(blk_char2, buf);
                blk_int_result[2*k] = outputDisk+1;
                blk_int_result[2*k+1] = -1;
                if (outputDisk%100 == R_BLOCK_NUM-1)   blk_int_result[2*k]=0;
                if (outputDisk%100 == S_BLOCK_NUM-1)   blk_int_result[2*k]=0;
                writeBlockToDisk(blk_char_result, outputDisk, buf);
                blk_char_result = getNewBlockInBuffer(buf);
                blk_int_result = (unsigned int*)blk_char_result;
                outputDisk++;
                k = 0;
            }
        }
        if (readDiskNum2 == diskNum && j == BLOCK_TUPLE_NUM) {
            if (i < BLOCK_TUPLE_NUM) {
                while (i < BLOCK_TUPLE_NUM) {
                    blk_int_result[2*k] = blk_int1[2*i];
                    blk_int_result[2*k+1] = blk_int1[2*i+1];
                    i++;
                    k++;
                }
                flag1 = 0;
                disk1++;
                freeBlockInBuffer(blk_char1, buf);
                blk_int_result[2*k] = outputDisk+1;
                blk_int_result[2*k+1] = -1;
                if (outputDisk%100 == R_BLOCK_NUM-1)   blk_int_result[2*k]=0;
                if (outputDisk%100 == S_BLOCK_NUM-1)   blk_int_result[2*k]=0;
                writeBlockToDisk(blk_char_result, outputDisk, buf);
                blk_char_result = getNewBlockInBuffer(buf);
                blk_int_result = (unsigned int*)blk_char_result;
                outputDisk++;
                k = 0;
            }
        }
        if (readDiskNum1 == diskNum && readDiskNum2 == diskNum) {
            freeBlockInBuffer(blk_char_result, buf);
            break;
        }
    }
}

void swap(int *p1, int *p2) {
    int temp = *p1;
    *p1 = *p2;
    *p2 = temp;
}


void sortDisk(int disk, Buffer *buf) {
    unsigned char *blk_char;
    unsigned int *blk_int;
    int i, j;
    blk_char = readBlockFromDisk(disk, buf);
    blk_int = (unsigned int *)blk_char;
    for (i = 0; i < BLOCK_TUPLE_NUM-1; i++) {
        for (j = i+1; j < BLOCK_TUPLE_NUM; j++) {
            if (blk_int[2*i] > blk_int[2*j]) {
                swap(blk_int+2*i, blk_int+2*j);
                swap(blk_int+2*i+1, blk_int+2*j+1);
            }
        }
    }
    writeBlockToDisk(blk_char, disk, buf);
}

void mergeSortAllDisk(Buffer *buf) {
    int i, j, k;
    int disk1, disk2, output;
    for (i = 0; i < R_BLOCK_NUM; i++) {
        sortDisk(R_BEGIN_DISK+i, buf);
    }
    for (i = 0; i < S_BLOCK_NUM; i++) {
        sortDisk(S_BEGIN_DISK+i, buf);
    }
    writeRS(R_BEGIN_DISK, S_BEGIN_DISK, buf);
    for (i = 1; i <= R_BLOCK_NUM/2; i=i*2) {
        for (j = 0; j < R_BLOCK_NUM/2/i; j++) {
            disk1 = R_BEGIN_DISK + 2 * i * j + (i-1)*1000;
            disk2 = disk1 + i;
            output = i*1000+disk1;
            if (i == R_BLOCK_NUM/2) output = R_BEGIN_DISK+2*i*j;
            printf("%d  %d  %d\n", disk1, disk2, output);
            mergeSort(disk1, disk2, i, output, buf);
        }
        printf("\n");
    }
    for (i = 1; i <= S_BLOCK_NUM/2; i=i*2) {
        for (j = 0; j < S_BLOCK_NUM/2/i; j++) {
            disk1 = S_BEGIN_DISK + 2 * i * j + (i-1)*2000;
            disk2 = disk1 + i;
            output = i*2000+disk1;
            if (i == S_BLOCK_NUM/2) output = S_BEGIN_DISK + 2*i*j;
            printf("%d  %d  %d\n", disk1, disk2, output);
            mergeSort(disk1, disk2, i, output, buf);
        }
        printf("\n");
    }
    for (i = 1; i < R_BLOCK_NUM/2; i=i*2) {
        for (j = 0; j < R_BLOCK_NUM/2/i; j++) {
            disk1 = R_BEGIN_DISK + 2 * i * j + (i-1)*1000;
            disk2 = disk1 + i;
            output = i*1000+disk1;
            for (k = disk1; k < disk1+2*i; k++) {
                dropBlockOnDisk(i*1000+k);
            }
        }
    }
    for (i = 1; i < S_BLOCK_NUM/2; i=i*2) {
        for (j = 0; j < S_BLOCK_NUM/2/i; j++) {
            disk1 = S_BEGIN_DISK + 2 * i * j + (i-1)*2000;
            disk2 = disk1 + i;
            output = i*2000+disk1;
            for (k = disk1; k < disk1+2*i; k++) {
                dropBlockOnDisk(i*2000+k);
            }
        }
    }
}

void binarySearch(int R_A, int S_C, int *buf) {
    int i, j;
    int k = 0;
    int searchDisk;
    int outputDisk = 10;
    unsigned char *blk_char = NULL;
    unsigned int *blk_int = NULL;
    unsigned char *result_blk_char = NULL;
    unsigned int *result_blk_int = NULL;
    result_blk_char = getNewBlockInBuffer(buf);
    result_blk_int = (unsigned int *)result_blk_char;
    searchDisk = R_BEGIN_DISK+R_BLOCK_NUM/2;
    while (1) {
        blk_char = readBlockFromDisk(searchDisk, buf);
        blk_int = (unsigned int*)blk_char;
        
    }
}