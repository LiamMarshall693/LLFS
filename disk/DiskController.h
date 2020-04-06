#include <stdio.h>
#include <stdlib.h>

#define MAX_BLOCKS 4096
#define BLOCK_SIZE 512

int writeBlock(void *input, int size, int num, int blockNum);

int readBlock(int blockNum, void* p);
