#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../disk/DiskController.h"
#include "../io/File.h"


int main(int argc, char *argv[])
{

	printf("Beginning test01\n");

	LLFS_Interface *LLFS = OpenLLFS();
	LLFS_Init(LLFS);

	char data[BLOCK_SIZE];
	memset(data, 0, 512);
	for (int i = 0; i < 255; i++) data[i] = i;
	char* name = "TestFile1";

	printf("Adding TestFile1\n");
	LLFS_new(LLFS, name, data, 512);

	for (int i = 0; i < 255; i++) data[i] = 255-i;
	name = "TestFile2.txt";

	printf("Adding TestFile2\n");
	LLFS_new(LLFS, name, data, 512);

	printf("Adding TestDirectory\n");
	name = "TestDirectory";
	LLFS_mkdir(LLFS, name);

	LLFS_ls(LLFS);

	char * path = "/TestDirectory/";

	printf("Moving to TestDirectory\n");
	LLFS_cd(LLFS, path);

	printf("Adding SubDirectory\n");
	name = "SubDirectory";
	LLFS_mkdir(LLFS, name);

	printf("Moving to SubDirectory");
	path = "/TestDirectory/SubDirectory/";
	LLFS_cd(LLFS, path);

	name = "SubSubDirectory";
	LLFS_mkdir(LLFS, name);

	LLFS_ls(LLFS);
	path = "/TestDirectory/SubDirectory/SubSubDirectory/";
	LLFS_cd(LLFS, path);



	path = "/";
	LLFS_cd(LLFS, path);
	printf("BACK TO ROOT\n");

	LLFS_ls(LLFS);

	name = "TestFile2.txt";
	for (int i = 0; i < 255; i++) data[i] = 'A';
	LLFS_modify(LLFS, name, data, BLOCK_SIZE);

	LLFS_flush(LLFS);

	//struct inode a = writeInode(512,10, 0);

}
