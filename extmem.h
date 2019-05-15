/*
 * extmem.c
 * Zhaonian Zou
 * Harbin Institute of Technology
 * Jun 22, 2011
 */

#ifndef EXTMEM_H
#define EXTMEM_H

#define BLOCK_AVAILABLE 0
#define BLOCK_UNAVAILABLE 1

typedef struct tagBuffer {
    unsigned long numIO; /* Number of IO's*/
    size_t bufSize; /* Buffer size*/
    size_t blkSize; /* Block size */
    size_t numAllBlk; /* Number of blocks that can be kept in the buffer */
    size_t numFreeBlk; /* Number of available blocks in the buffer */
    unsigned char *data; /* Starting address of the buffer */
} Buffer;

Buffer *initBuffer(size_t bufSize, size_t blkSize, Buffer *buf);

void freeBuffer(Buffer *buf);

unsigned char *getNewBlockInBuffer(Buffer *buf);

void freeBlockInBuffer(unsigned char *blk, Buffer *buf);

int dropBlockOnDisk(unsigned int addr);

unsigned char *readBlockFromDisk(unsigned int addr, Buffer *buf);

int writeBlockToDisk(unsigned char *blkPtr, unsigned int addr, Buffer *buf);

#endif