#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../disk/DiskController.h"
#include "../io/File.h"


int main(int argc, char *argv[])
{

	printf("Beginning test02\n");

	LLFS_Interface *LLFS = OpenLLFS();

	printf("Testing ls:\n");
	LLFS_ls(LLFS);

	char data[BLOCK_SIZE];
	memset(data, 0, BLOCK_SIZE);

	char *name = "TestFile1";
	for (int i = 0; i < 255; i++) data[i] = 'A';
	LLFS_modify(LLFS, name, data, BLOCK_SIZE);

	char* data_small = "testing...";
	name = "TestFile3.txt";
	LLFS_new(LLFS, name, data_small, strlen(data_small));
	name = "TestFile4.txt";
	LLFS_new(LLFS, name, data_small, strlen(data_small));
	name = "TestFile5.txt";
	LLFS_new(LLFS, name, data_small, strlen(data_small));
	name = "TestFile6.txt";
	LLFS_new(LLFS, name, data_small, strlen(data_small));
	name = "TestFile7.txt";
	LLFS_new(LLFS, name, data_small, strlen(data_small));
	name = "TestFile8.txt";
	LLFS_new(LLFS, name, data_small, strlen(data_small));
	name = "TestFile9.txt";
	LLFS_new(LLFS, name, data_small, strlen(data_small));
	name = "TestFile10.txt";
	LLFS_new(LLFS, name, data_small, strlen(data_small));
	name = "TestFile11.txt";
	LLFS_new(LLFS, name, data_small, strlen(data_small));
	name = "TestFile12.txt";
	LLFS_new(LLFS, name, data_small, strlen(data_small));
	name = "TestFile13.txt";
	LLFS_new(LLFS, name, data_small, strlen(data_small));
	name = "TestFile14.txt";
	LLFS_new(LLFS, name, data_small, strlen(data_small));
	name = "TestFile15.txt";
	LLFS_new(LLFS, name, data_small, strlen(data_small));
	name = "TestFile16.txt";
	LLFS_new(LLFS, name, data_small, strlen(data_small));
	name = "TestFile17.txt";
	LLFS_new(LLFS, name, data_small, strlen(data_small));

	LLFS_flush(LLFS);

	LLFS_ls(LLFS);

	printf("now removing TestFile3\n");
	name = "TestFile3.txt";
	LLFS_rm(LLFS, name);


	LLFS_ls(LLFS);

}
