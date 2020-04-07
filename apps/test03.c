#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../disk/DiskController.h"
#include "../io/File.h"


int main(int argc, char *argv[])
{

	printf("Beginning test03\n");

	LLFS_Interface *LLFS = OpenLLFS();

	printf("Corrupting free block vector...\n");
	//simulates corruption by randomly setting bits in the free block vector that
	//should be cleared.
	char block[BLOCK_SIZE];
	memset(block, 255, BLOCK_SIZE);
	block[1] = 0b011101101;
	writeBlock(block, BLOCK_SIZE, 1, 1);

	//currently commented, hexdump will display corruption in block 1 (00000200)
	//uncomment to see it fixed

	//printf("Fixing free block vector
	LLFS_fsck(LLFS);


	LLFS_flush(LLFS);

}
