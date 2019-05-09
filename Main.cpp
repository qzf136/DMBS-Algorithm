#include <stdio.h>
#include <stdlib.h>
#include "select.cpp"

int main(int argc, char const *argv[])
{
    Buffer buf;
    initBuffer(bufSize, blkSize, &buf);
    generateRS(&buf);
    writeRS(R_BEGIN_DISK, S_BEGIN_DISK, &buf);
    // linearSelect(40, 60, &buf);
    // printBlock(0, &buf);
    // printBlock(1, &buf);

    mergeSortAllDisk(&buf);
    writeRS(R_BEGIN_DISK, S_BEGIN_DISK, &buf);
    // binarySearch(40, 60, &buf);
    // printBlock(10, &buf);
    // printBlock(11, &buf);
    return 0;
}