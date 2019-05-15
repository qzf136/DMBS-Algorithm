#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>
#include <set>
#include "util.h"

using namespace std;

typedef int KEY;
typedef set<int> VALUE;

typedef struct bplus_node_s* bplus_node_pt;

typedef struct bplus_node_s {
    int keynum;
    KEY *keys;            /* 主健，最大max个,最小min个, 空间max+1*/
    VALUE **data;          /* 数据，最大max个,最小min个, 空间max+1，内部节点时，data是NULL */
    bplus_node_pt *child; /* 子节点，最大max+1个，最小min + 1个, 空间max+2，叶子节点时，child是NULL */
    bplus_node_pt parent;
    bplus_node_pt next;   /* 兄弟节点，仅是叶子节点时有值 */
} bplus_node_t;

typedef struct bplus_tree_s* bplus_tree_pt;

typedef struct bplus_tree_s {
    bplus_node_pt root;
    int max;
    int min;
} bplus_tree_t;

#define item_cmp(a, b) ((a) - (b))

set<int> setArray[500];
int setID = 0;

static int bplus_tree_create(bplus_tree_pt *_tree, int m);
static int bplus_tree_insert(bplus_tree_pt tree, KEY key, int val);
static VALUE *bplus_tree_search(bplus_tree_pt tree, KEY key);
static void bplus_tree_print(bplus_tree_pt tree);

static bplus_node_pt bplus_node_new_leaf(int m);
static bplus_node_pt bplus_node_new_internal(int m);
static int binary_search(KEY *keys, KEY key, int left, int right, int *index);
static int _bplus_tree_insert(bplus_tree_pt tree, bplus_node_pt node, KEY key, int val);

static void mergeSort(int begin1, int begin2, int diskNum, int output, Buffer *buf);
static void swap(unsigned int *p1, unsigned int *p2);
static void sortDisk(int disk, Buffer *buf);
static int* rightSearchEqual(int val, int disk, int *outInfo, Buffer *buf);
static int* leftSearchEqual(int val, int disk, int *outInfo, Buffer *buf);
static int* rightSearchNequal(int val, int disk, int* outInfo, Buffer *buf);

static void mergeSort(int begin1, int begin2, int diskNum, int output, Buffer *buf) {
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

static void swap(unsigned int *p1, unsigned int *p2) {
    int temp = *p1;
    *p1 = *p2;
    *p2 = temp;
}

static void sortDisk(int disk, Buffer *buf) {
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

static int* rightSearchEqual(int val, int disk, int *outInfo, Buffer *buf) {
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

static int* leftSearchEqual(int val, int disk, int *outInfo, Buffer *buf) {
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

static int* rightSearchNequal(int val, int disk, int* outInfo, Buffer *buf) {
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

static int bplus_tree_create(bplus_tree_pt *_tree, int m) {
    bplus_tree_pt tree = (bplus_tree_pt)malloc(sizeof(bplus_tree_t));
    if (tree == NULL) {
        return 0;
    }
    tree->root = NULL;
    tree->max = m;
    tree->min = m/2;
    *_tree = tree;
    return 1;
}

/* 
 * Name:  bplus_node_new_leaf
 * Description:  创建新的叶子节点
 */
static bplus_node_pt bplus_node_new_leaf(int m) {
    bplus_node_pt node = (bplus_node_pt)malloc(sizeof(bplus_node_t));
    if (node == NULL) {
        return NULL;
    }
    node->parent = NULL;
    node->next = NULL;
    node->keynum = 0;
    node->keys = (KEY *)malloc(sizeof(KEY)*(m + 1));
    node->data = (VALUE **)malloc(sizeof(VALUE*)*(m + 1));
    for (int i = 0; i < m+1; i++) {
        node->data[i] = setArray+setID;
        setID++;
    }
    if (node->keys == NULL || node->data == NULL) {
        free(node->keys);
        free(node->data);
        free(node);
        return NULL;
    }
    node->child = NULL;
    return node;
}

/* 
 * Name:  bplus_node_new_internal
 * Description:  创建新的内部节点
 * 
 */
static bplus_node_pt bplus_node_new_internal(int m) {
    bplus_node_pt node = (bplus_node_pt)malloc(sizeof(bplus_node_t));
    if (node == NULL) {
        return NULL;
    }
    node->parent = NULL;
    node->next = NULL;
    node->keynum = 0;
    node->keys = (KEY *)malloc(sizeof(KEY)*(m + 1));
    node->data = NULL;
    node->child = (bplus_node_pt *)malloc(sizeof(bplus_node_pt)*(m + 2));
    if (node->keys == NULL || node->child == NULL) {
        free(node->keys);
        free(node->child);
        free(node);
        return NULL;
    }
    return node;
}

/* 
 * Name:  binary_search
 * Description:  二分查找，找到数组中指定key的位置，如果存在，返回1, 否则返回0
 *               如果找到索引为该值位置，否则返回右邻值位置(child的位置)
 * 
 */
static int binary_search(KEY *keys, KEY key, int left, int right, int *index) {
    int mid;
    while (left <= right) {
        mid = (left + right)/2;
        if (item_cmp(keys[mid], key) > 0) {
            right = mid - 1;
        }
        else if (item_cmp(keys[mid], key) < 0) {
            left = mid + 1;
        }
        else {
            *index = mid;
            return 1;
        }
    }
    *index = left;
    return 0;
}


/* 
 * Name:  _bplus_tree_insert
 * Description:  元素插入及分裂操作
 * 
 */
static int _bplus_tree_insert(bplus_tree_pt tree, bplus_node_pt node, KEY key, int val)
{
    KEY temp;
    bplus_node_pt parent, node2;
    int i, mid;

    /* 插入叶子节点 */
    for (i=node->keynum; i>0 && item_cmp(node->keys[i-1], key) > 0; i--) {
        node->keys[i] = node->keys[i - 1];
        node->data[i] = node->data[i - 1];
    }
    node->keys[i] = key;
    (*node->data[i]).insert(val);
    node->keynum++;
    while (node->keynum > tree->max) {

        /* 分裂节点 */
        if (node->child == NULL) {
            /* 叶子节点分裂 */
            node2 = bplus_node_new_leaf(tree->max);
        }
        else {
            /* 内部节点分裂 */
            node2 = bplus_node_new_internal(tree->max);
        }
        if (node2 == NULL) {
            return 0;
        }

        /* 拷贝数据 */
        mid = node->keynum/2;
        temp = node->keys[mid];
        if (node->child == NULL) {
            node2->keynum = node->keynum - mid;
            memcpy(node2->keys, node->keys + mid, sizeof(KEY)*(node2->keynum));
            memcpy(node2->data, node->data + mid, sizeof(KEY)*(node2->keynum));
            node2->next = node->next;
            node->next = node2;
        }
        else {
            node2->keynum = node->keynum - mid - 1;
            memcpy(node2->keys, node->keys + mid + 1, sizeof(KEY)*(node2->keynum));
            memcpy(node2->child, node->child + mid + 1, sizeof(bplus_node_pt)*(node->keynum - mid));
            /* 重设父指针 */
            for (i=0; i<=node2->keynum; i++) {
                node2->child[i]->parent = node2;
            }
        }
        node->keynum = mid;

        /* 插入父节点 */
        parent = node->parent;
        if (parent == NULL) {
            /* 生成新的树根节点 */
            parent = bplus_node_new_internal(tree->max);
            if (parent == NULL) {
                return 0;
            }
            parent->child[0] = node;
            node->parent = parent;
            tree->root = parent;
        }
        /* 增加数据和右子树 */
        for (i=parent->keynum; i>0 && item_cmp(parent->keys[i-1], temp) > 0; i--) {
            parent->keys[i] = parent->keys[i - 1];
            parent->child[i + 1] = parent->child[i];
        }
        parent->keys[i] = temp;
        parent->child[i + 1] = node2;
        parent->keynum++;

        node2->parent = parent;
        node = parent;
    }

    return 1;
}

static int bplus_tree_insert(bplus_tree_pt tree, KEY key, int val) {
    bplus_node_pt node;
    int ret, index;
    if (tree->root == NULL) {
        node = bplus_node_new_leaf(tree->max);
        if (node == NULL) {
            return 0;
        }
        tree->root = node;
    }
    node = tree->root;
    /* 查找到叶节点 */
    while (node->child != NULL) {
        ret = binary_search(node->keys, key, 0, node->keynum - 1, &index);
        if (ret == 1) {
            index++;
        }
        node = node->child[index];
    }
    ret = binary_search(node->keys, key, 0, node->keynum - 1, &index);
    if (ret == 1) {
        (*node->data[index]).insert(val);
        return 0;
    }
    _bplus_tree_insert(tree, node, key, val);
    return 1;
}

static VALUE *bplus_tree_search(bplus_tree_pt tree, KEY key) {
    bplus_node_pt node = tree->root;
    int ret = 0, index;
    if (node == NULL) {
        return NULL;
    }
    ret = binary_search(node->keys, key, 0, node->keynum - 1, &index);
    while (node->child != NULL) {
        if (ret == 1) {
            index++;
        }
        node = node->child[index];
        ret = binary_search(node->keys, key, 0, node->keynum - 1, &index);
    }
    if (ret == 0) {
        return NULL;
    }
    return &(*node->data[index]);
}

static void bplus_node_printnode(KEY *key, int h) {
    int i;
    for (i=0; i < h; i++) {
        printf("    ");
    }
    if (key == NULL) {
        printf("*\n");
    }
    else {
        printf("%d\n", *key);
    }
}

static void bplus_node_show(bplus_node_pt node, int h) {
    int i;

    if (node == NULL) {
        return;
    }

    if (node->child != NULL) {
        bplus_node_show(node->child[0], h + 1);
    }
    for (i=0; i<node->keynum; i++) {
        bplus_node_printnode(&node->keys[i], h);
        if (node->child != NULL) {
            bplus_node_show(node->child[i + 1], h + 1);
        }
    }
}

static void bplus_tree_print(bplus_tree_pt tree) {
    bplus_node_show(tree->root, 0);
}

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
    int outputDisk = 0;
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

void BPlusTreeSearch(int R_A, int S_C, Buffer *buf) {
    int i, j;
    bplus_tree_pt tree;
    bplus_tree_create(&tree, 5);
    unsigned char *blk_char;
    unsigned int *blk_int;
    for (i = 0; i < R_BLOCK_NUM; i++) {
        blk_char = readBlockFromDisk(R_BEGIN_DISK+i, buf);
        blk_int = (unsigned int *)blk_char;
        for (j = 0; j < BLOCK_TUPLE_NUM; j++) {
            bplus_tree_insert(tree, blk_int[2*j], R_BEGIN_DISK+i);
        }
        freeBlockInBuffer(blk_char, buf);
    }
    bplus_tree_pt tree2;
    bplus_tree_create(&tree2, 5);
    for (i = 0; i < S_BLOCK_NUM; i++) {
        blk_char = readBlockFromDisk(S_BEGIN_DISK+i, buf);
        blk_int = (unsigned int *)blk_char;
        for (j = 0; j < BLOCK_TUPLE_NUM; j++) {
            bplus_tree_insert(tree2, blk_int[2*j], S_BEGIN_DISK+i);
        }
        freeBlockInBuffer(blk_char, buf);
    }
    int k = 0;
    int DISK = 0;
    unsigned char *result_blk_char = getNewBlockInBuffer(buf);
    unsigned int *result_blk_int = (unsigned int *)result_blk_char;
    VALUE *find = bplus_tree_search(tree, R_A);
    if (find != NULL) {
        set<int>::iterator it;
        for (it=(*find).begin(); it != (*find).end(); it++) {
            blk_char = readBlockFromDisk(*it, buf);
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
    }
    find = bplus_tree_search(tree2, S_C);
    if (find != NULL) {
        set<int>::iterator it;
        for (it=(*find).begin(); it != (*find).end(); it++) {
            blk_char = readBlockFromDisk(*it, buf);
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