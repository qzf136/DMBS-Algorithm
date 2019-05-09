#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>
#include "util.h"

using namespace std;

map<unsigned int, int> RleftBound;
map<unsigned int, int> RrightBound;
map<unsigned int, int> SleftBound;
map<unsigned int, int> SrightBound;

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
                result_blk_int[2*k+1] = DISK-1;
                if (DISK == 0)  result_blk_int[2*k+1] = 0;
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
                result_blk_int[2*k+1] = DISK-1;
                if (DISK == 0)  result_blk_int[2*k+1] = 0;
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
        result_blk_int[2*k+1] = DISK-1;
        if (DISK == 0)  result_blk_int[2*k+1] = 0;
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
                blk_int_result[2*k+1] = outputDisk-1;
                if (outputDisk%100 == R_BLOCK_NUM-1)   blk_int_result[2*k]=0;
                if (outputDisk%100 == S_BLOCK_NUM-1)   blk_int_result[2*k]=0;
                if (outputDisk%1000 == 100) blk_int_result[2*k+1] = 0;
                if (outputDisk%2000 == 200) blk_int_result[2*k+1] = 0;
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
                blk_int_result[2*k+1] = outputDisk-1;
                if (outputDisk%100 == R_BLOCK_NUM-1)   blk_int_result[2*k]=0;
                if (outputDisk%100 == S_BLOCK_NUM-1)   blk_int_result[2*k]=0;
                if (outputDisk%1000 == 100) blk_int_result[2*k+1] = 0;
                if (outputDisk%2000 == 200) blk_int_result[2*k+1] = 0;
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
                blk_int_result[2*k+1] = outputDisk-1;
                if (outputDisk%100 == R_BLOCK_NUM-1)   blk_int_result[2*k]=0;
                if (outputDisk%100 == S_BLOCK_NUM-1)   blk_int_result[2*k]=0;
                if (outputDisk%1000 == 100) blk_int_result[2*k+1] = 0;
                if (outputDisk%2000 == 200) blk_int_result[2*k+1] = 0;
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

void swap(unsigned int *p1, unsigned int *p2) {
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
            mergeSort(disk1, disk2, i, output, buf);
        }
    }
    for (i = 1; i <= S_BLOCK_NUM/2; i=i*2) {
        for (j = 0; j < S_BLOCK_NUM/2/i; j++) {
            disk1 = S_BEGIN_DISK + 2 * i * j + (i-1)*2000;
            disk2 = disk1 + i;
            output = i*2000+disk1;
            if (i == S_BLOCK_NUM/2) output = S_BEGIN_DISK + 2*i*j;
            mergeSort(disk1, disk2, i, output, buf);
        }
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

void binarySearch(int R_A, int S_C, Buffer *buf) {
    int *outInfo  = (int *)malloc(sizeof(int)*2);
    int i, j;
    int k = 0;
    int readData;
    int searchDisk, leftDisk, rightDisk;
    int outputDisk = 10;
    int thisSearch;
    unsigned char *blk_char = NULL;
    unsigned int *blk_int = NULL;
    searchDisk = R_BEGIN_DISK+R_BLOCK_NUM/2;
    leftDisk = R_BEGIN_DISK;
    rightDisk = R_BEGIN_DISK + R_BLOCK_NUM - 1;
    while (1) {
        thisSearch = searchDisk;
        blk_char = readBlockFromDisk(searchDisk, buf);
        blk_int = (unsigned int*)blk_char;
        readData = blk_int[0];
        if (readData < R_A) {
            freeBlockInBuffer(blk_char, buf);
            leftDisk = searchDisk;
            searchDisk = (searchDisk + rightDisk) / 2;
        } else if (readData > R_A) {
            freeBlockInBuffer(blk_char, buf);
            rightDisk = searchDisk;
            searchDisk = (searchDisk + leftDisk) / 2;
        } else {
            outInfo[0] = outputDisk;
            outInfo[1] = k;
            outInfo = leftSearchEqual(R_A, searchDisk, outInfo, buf);
            outInfo = rightSearchEqual(R_A, searchDisk, outInfo, buf);
            break;
        }
        if (thisSearch == searchDisk) {
            outInfo[0] = outputDisk;
            outInfo[1] = k;
            outInfo = rightSearchNequal(R_A, searchDisk, outInfo, buf);
            break;
        }
    }
    freeBlockInBuffer(blk_char, buf);
    searchDisk = S_BEGIN_DISK+S_BLOCK_NUM/2;
    leftDisk = S_BEGIN_DISK;
    rightDisk = S_BEGIN_DISK + S_BLOCK_NUM - 1;
    outputDisk = outInfo[0];
    k = outInfo[1];
    while (1) {
        thisSearch = searchDisk;
        blk_char = readBlockFromDisk(searchDisk, buf);
        blk_int = (unsigned int*)blk_char;
        readData = blk_int[0];
        if (readData < S_C) {
            freeBlockInBuffer(blk_char, buf);
            leftDisk = searchDisk;
            searchDisk = (searchDisk + rightDisk) / 2;
        } else if (readData > S_C) {
            freeBlockInBuffer(blk_char, buf);
            rightDisk = searchDisk;
            searchDisk = (searchDisk + leftDisk) / 2;
        } else {
            outInfo[0] = outputDisk;
            outInfo[1] = k;
            outInfo = leftSearchEqual(S_C, searchDisk, outInfo, buf);
            outInfo = rightSearchEqual(S_C, searchDisk, outInfo, buf);
            break;
        }
        if (thisSearch == searchDisk) {
            outInfo[0] = outputDisk;
            outInfo[1] = k;
            outInfo = rightSearchNequal(S_C, searchDisk, outInfo, buf);
            
            break;
        }
    }
    outputDisk = outInfo[0];
    freeBlockInBuffer(blk_char, buf);
    blk_char = readBlockFromDisk(outputDisk, buf);
    blk_int = (unsigned int *)blk_char;
    blk_int[2*BLOCK_TUPLE_NUM+1] = outputDisk-1;
    writeBlockToDisk(blk_char, outputDisk, buf);
}

int* rightSearchEqual(int val, int disk, int *outInfo, Buffer *buf) {
    int i = 0;
    unsigned char *blk_char = readBlockFromDisk(disk, buf);
    unsigned int *blk_int = (unsigned int *)blk_char;
    unsigned char *result_blk_char;
    unsigned int *result_blk_int;
    int outputDisk = outInfo[0];
    int k = outInfo[1];
    if (k != 0)     result_blk_char = readBlockFromDisk(outputDisk, buf);
    else    result_blk_char = getNewBlockInBuffer(buf);
    int nextReadDisk;
    while (1) {
        while (blk_int[2*i] == val && i < BLOCK_TUPLE_NUM) {
            result_blk_int[2*k] = blk_int[2*i];
            result_blk_int[2*k+1] = blk_int[2*i+1];
            i++;
            k++;
            if (k == BLOCK_TUPLE_NUM) {
                // result_blk_int[2*k] = outputDisk+1;
                // result_blk_int[2*k+1] = outputDisk-1;
                // if (outputDisk == 10) result_blk_int[2*k+1] = 0;
                writeBlockToDisk(result_blk_char, outputDisk, buf);
                result_blk_char = getNewBlockInBuffer(buf);
                result_blk_int = (unsigned int *)result_blk_char;
                k = 0;
                outputDisk++;
            }
        }
        if (i == BLOCK_TUPLE_NUM) {
            nextReadDisk = blk_int[2*BLOCK_TUPLE_NUM];
            freeBlockInBuffer(blk_char, buf);
            blk_char = readBlockFromDisk(nextReadDisk,buf);
            blk_int = (unsigned int *)blk_char;
            i = 0;
        }
        if (blk_int[2*i] != val) {
            freeBlockInBuffer(blk_char, buf);
            break;
        }
    }
    if (k != BLOCK_TUPLE_NUM) {
        memset(result_blk_char+4*2*k, 0, sizeof(char)*2*(BLOCK_TUPLE_NUM+1-k)*4);
        writeBlockToDisk(result_blk_char, outputDisk, buf);
        freeBlockInBuffer(result_blk_char, buf);
    }
    freeBlockInBuffer(result_blk_char, buf);
    freeBlockInBuffer(blk_char, buf);
    outInfo[0] = outputDisk;
    outInfo[1] = k;
    return outInfo;
}

int* leftSearchEqual(int val, int disk, int *outInfo, Buffer *buf) {
    int i = BLOCK_TUPLE_NUM-1;
    unsigned char *blk_char = readBlockFromDisk(disk, buf);
    unsigned int *blk_int = (unsigned int *)blk_char;
    unsigned char *result_blk_char;
    unsigned int *result_blk_int;
    int outputDisk = outInfo[0];
    int k = outInfo[1];
    if (k != 0)     result_blk_char = readBlockFromDisk(outputDisk, buf);
    else    result_blk_char = getNewBlockInBuffer(buf);
    int readDisk = blk_int[2*BLOCK_TUPLE_NUM+1];
    freeBlockInBuffer(blk_char, buf);
    if (readDisk == 0)  return outInfo;
    blk_char = readBlockFromDisk(readDisk, buf);
    blk_int = (unsigned int *)blk_char;
    while (1) {
        while (blk_int[2*i] == val && i >= 0) {
            result_blk_int[2*k] = blk_int[2*i];
            result_blk_int[2*k+1] = blk_int[2*i+1];
            i--;
            k++;
            if (k == BLOCK_TUPLE_NUM) {
                // result_blk_int[2*k] = outputDisk+1;
                // result_blk_int[2*k+1] = outputDisk-1;
                // if (outputDisk == 10) result_blk_int[2*k+1] = 0;
                writeBlockToDisk(result_blk_char, outputDisk, buf);
                result_blk_char = getNewBlockInBuffer(buf);
                result_blk_int = (unsigned int *)result_blk_char;
                k = 0;
                outputDisk++;
            }
        }
        if (i == -1) {
            readDisk = blk_int[2*BLOCK_TUPLE_NUM+1];
            freeBlockInBuffer(blk_char, buf);
            blk_char = readBlockFromDisk(readDisk,buf);
            blk_int = (unsigned int *)blk_char;
            i = BLOCK_TUPLE_NUM-1;
        }
        if (blk_int[2*i] != val) {
            freeBlockInBuffer(blk_char, buf);
            break;
        }
    }
    if (k != BLOCK_TUPLE_NUM) {
        // result_blk_int[2*k] = 0;
        // result_blk_int[2*k+1] = outputDisk-1;
        // if (outputDisk == 10) result_blk_int[2*k+1] = 0;
        // k++;
        memset(result_blk_char+4*2*k, 0, sizeof(char)*2*(BLOCK_TUPLE_NUM+1-k)*4);
        writeBlockToDisk(result_blk_char, outputDisk, buf);
        freeBlockInBuffer(result_blk_char, buf);
    }
    freeBlockInBuffer(result_blk_char, buf);
    freeBlockInBuffer(blk_char, buf);
    outInfo[0] = outputDisk;
    outInfo[1] = k;
    return outInfo;
}

int* rightSearchNequal(int val, int disk, int* outInfo, Buffer *buf) {
    int outDisk = outInfo[0];
    int k = outInfo[1];
    int readDisk = disk;
    unsigned char *blk_char, *result_blk_char;
    unsigned int *blk_int, *result_blk_int;
    if (k != 0)     result_blk_char = readBlockFromDisk(outDisk, buf);
    else    result_blk_char = getNewBlockInBuffer(buf);
    result_blk_int = (unsigned int *)result_blk_char;
    int i = 0;
    blk_char = readBlockFromDisk(readDisk, buf);
    blk_int = (unsigned int *)blk_char;
    while (blk_int[2*i] != val) {
        i++;
        if (i == BLOCK_TUPLE_NUM) {
            i = 0;
            freeBlockInBuffer(blk_char, buf);
            readDisk = blk_int[2*BLOCK_TUPLE_NUM];
            if (readDisk == 0)   break;
            blk_char = readBlockFromDisk(readDisk, buf);
            blk_int = (unsigned int *)blk_char;
        }
    }
    while (blk_int[2*i] == val) {
        result_blk_int[2*k] = blk_int[2*i];
        result_blk_int[2*k+1] = blk_int[2*i+1];
        i++;
        k++;
        if (k == BLOCK_TUPLE_NUM) {
            result_blk_int[2*k] = outDisk+1;
            result_blk_int[2*k+1] = outDisk-1;
            if (outDisk == 10)  result_blk_int[2*k+1] = 0;
            writeBlockToDisk(result_blk_char, outDisk, buf);
            freeBlockInBuffer(result_blk_char, buf);
            k = 0;
            outDisk++;
            result_blk_char = getNewBlockInBuffer(buf);
            result_blk_int = (unsigned int *)result_blk_char;
        }
        if (i == BLOCK_TUPLE_NUM) {
            i = 0;
            freeBlockInBuffer(blk_char, buf);
            readDisk = blk_int[2*BLOCK_TUPLE_NUM];
            if (readDisk == 0)   break;
            blk_char = readBlockFromDisk(readDisk, buf);
            blk_int = (unsigned int *)blk_char;
        }
    }
    if (k != BLOCK_TUPLE_NUM) {
        memset(result_blk_char+4*2*k, 0, sizeof(char)*2*(BLOCK_TUPLE_NUM+1-k)*4);
        writeBlockToDisk(result_blk_char, outDisk, buf);
        freeBlockInBuffer(result_blk_char, buf);
    }
    outInfo[0] = outDisk;
    outInfo[1] = k;
    freeBlockInBuffer(result_blk_char, buf);
    freeBlockInBuffer(blk_char, buf);
    return outInfo;
}

void constructMap(Buffer *buf) {
    int i, j;
    int v;
    unsigned char *blk_char;
    unsigned int *blk_int;
    for (i = 0; i < R_BLOCK_NUM; i++) {
        blk_char = readBlockFromDisk(R_BEGIN_DISK+i, buf);
        blk_int = (unsigned int *)blk_char;
        for (j = 0; j < BLOCK_TUPLE_NUM; j++) {
            v = blk_int[2*j];
            if (RleftBound[v] == 0) RleftBound[v] = R_BEGIN_DISK+i;
            if (R_BEGIN_DISK+i > RrightBound[v])    RrightBound[v] = R_BEGIN_DISK+i;
        }
        freeBlockInBuffer(blk_char, buf);
    }
    for (i = 0; i < S_BLOCK_NUM; i++) {
        blk_char = readBlockFromDisk(S_BEGIN_DISK+i, buf);
        blk_int = (unsigned int *)blk_char;
        for (j = 0; j < BLOCK_TUPLE_NUM; j++) {
            v = blk_int[2*j];
            if (SleftBound[v] == 0) SleftBound[v] = S_BEGIN_DISK+i;
            if (S_BEGIN_DISK+i > SrightBound[v])    SrightBound[v] = S_BEGIN_DISK+i;
        }
        freeBlockInBuffer(blk_char, buf);
    }
    
    // for (iter = RrightBound.begin(); iter != RrightBound.end(); iter++) {
    //     map<unsigned int, int>::iterator iter;
    //     cout << iter->first << "  " << iter->second << endl;
    // }
}