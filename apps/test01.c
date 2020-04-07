#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../disk/DiskController.h"
#include "../io/File.h"


int main(int argc, char *argv[])
{

	printf("Beginning test01\n");

	LLFS_Interface *LLFS = OpenLLFS();
	printf("Open complete\n");

	LLFS_Init(LLFS);
	printf("Init complete\n");

	char data[BLOCK_SIZE];
	memset(data, 0, 512);
	for (int i = 0; i < 255; i++) data[i] = i;
	char* name = "TestFile1";

	printf("Adding TestFile1\n");
	LLFS_new(LLFS, name, data, 512);

	name = "TestFile2.txt";

	//11 blocks needed, so demonstrates single indirect block
	char databigger[5632];
	for (int i = 0; i < 5632; i++) databigger[i] = 119;
	printf("Adding TestFile2\n");
	LLFS_new(LLFS, name, databigger, 5632);

	printf("Adding TestDirectory\n");
	name = "TestDirectory";
	LLFS_mkdir(LLFS, name);

	printf("Testing ls:\n");
	LLFS_ls(LLFS);

	char * path = "/TestDirectory/";

	printf("Moving to TestDirectory\n");
	LLFS_cd(LLFS, path);

	printf("Adding SubDirectory\n");
	name = "SubDirectory";
	LLFS_mkdir(LLFS, name);

	printf("Moving to SubDirectory\n");
	path = "/TestDirectory/SubDirectory/";
	LLFS_cd(LLFS, path);

	printf("Adding SubSubDirectory\n");
	name = "SubSubDirectory";
	LLFS_mkdir(LLFS, name);

	printf("Testing ls:\n");
	LLFS_ls(LLFS);
	path = "/TestDirectory/SubDirectory/SubSubDirectory/";
	LLFS_cd(LLFS, path);

	printf("Done test01\n");
}
