#include <stdlib.h>
#include "extmem.h"
#include "util.h"

using namespace std;

int main(int argc, char const *argv[])
{
    Buffer buf;
    initBuffer(BUFSIZE, BLKSIZE, &buf);
    generateRS(&buf);
    // writeRS(R_BEGIN_DISK, S_BEGIN_DISK, &buf);
    // linearSelect(40, 60, &buf);
    // printResult(0, &buf);

    // mergeSortAllDisk(&buf);
    // writeRS(R_BEGIN_DISK, S_BEGIN_DISK, &buf);
    // binarySearch(40, 60, &buf);
    // printResult(0, &buf);
    
    // BPlusTreeSearch(40, 60, &buf);
    // printResult(0, &buf);

    // Project(&buf);
    // printResult(0, &buf);

    // nestLoopJoin(&buf);
    // printResult(0, &buf);

    sortMergeJoin(&buf);
    printResult(0, &buf);
    return 0;
}