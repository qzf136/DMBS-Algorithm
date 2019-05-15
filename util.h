#include "extmem.h"

#ifndef UTIL_H
#define UTIL_H

# define BLKSIZE  64
# define BUFSIZE  520

# define TUPLE_SIZE  8
# define BLOCK_TUPLE_NUM  7

# define R_BLOCK_NUM  16
# define S_BLOCK_NUM  32

# define R_BEGIN_DISK  100
# define S_BEGIN_DISK  200


void generateRS(Buffer *buf);

void printBlock(int disk, Buffer *buf);

void printResult(int begin, Buffer *buf);

void writeRS(int R_beign, int S_begin, Buffer *buf);

void linearSelect(int R_A, int S_C, Buffer *buf);

void mergeSortAllDisk(Buffer *buf);

void binarySearch(int R_A, int S_C, Buffer *buf);

void BPlusTreeSearch(int R_A, int S_C, Buffer *buf);

void Project(Buffer *buf);

#endif