#include <stdio.h>
#include <stdlib.h>


#define DATA_BLOCKS 4078
#define MAX_INODES 255
#define MAGIC_NUM 1397115980

typedef struct {
	char* buffer;
	int blocks;
	int inodes;
	int size;
	char* path;
	short int path_len;
	short int* updates;

} LLFS_Interface;

int LLFS_rm(LLFS_Interface *LLFS_i, char* name);
int LLFS_ls(LLFS_Interface *LLFS_i);
int LLFS_cd(LLFS_Interface *LLFS_i, char* path);
int LLFS_mkdir(LLFS_Interface *LLFS_i, char* name);
int LLFS_flush(LLFS_Interface *LLFS_i);
int LLFS_modify(LLFS_Interface *LLFS_i, char* name, char* data, int size);
int LLFS_new(LLFS_Interface *LLFS_i, char* name, char* data, int size);
int LLFS_fsck(LLFS_Interface *LLFS_i);
LLFS_Interface* OpenLLFS();
int LLFS_Init(LLFS_Interface *LLFS_i);
