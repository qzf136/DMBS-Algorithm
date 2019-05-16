#include <stdio.h>
#include <stdlib.h>
#include "extmem.h"
#include "util.h"

using namespace std;

int main(int argc, char const *argv[])
{
    Buffer buf;
    initBuffer(BUFSIZE, BLKSIZE, &buf);
    int cmd;
    generateRS(&buf);
    writeRS(R_BEGIN_DISK, S_BEGIN_DISK, &buf);
    while (1) {
        printf("0: exit\n");
        printf("1: 线性搜索\n");
        printf("2: 二分搜索\n");
        printf("3: B+树搜索\n");
        printf("4: 投影\n");
        printf("5: NestLoopJoin\n");
        printf("6: SortMergeJoin\n");
        printf("7: HashJoin\n");
        printf("Input the cmd: ");
        scanf("%d", &cmd);
        switch (cmd)
        {
        case 0:
            return 0;
        case 1:
            linearSelect(40, 60, &buf);
            printResult(0, &buf);
            break;
        case 2:
            mergeSortAllDisk(&buf);
            writeRS(R_BEGIN_DISK, S_BEGIN_DISK, &buf);
            binarySearch(40, 60, &buf);
            printResult(0, &buf);
            break;
        case 3:
            BPlusTreeSearch(40, 60, &buf);
            printResult(0, &buf);
            break;
        case 4:
            Project(&buf);
            printResult(0, &buf);
            break;
        case 5:
            nestLoopJoin(&buf);
            printResult(0, &buf);
            break;
        case 6:
            sortMergeJoin(&buf);
            printResult(0, &buf);
            break;
        case 7:
            HashJoin(&buf);
            printResult(0, &buf);
            break;
        default:
            break;
        }
    }
    return 0;
}