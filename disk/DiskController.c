#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "DiskController.h"

#define TESTING


FILE* openVDisk(){
	FILE* fp;
	fp = fopen("../disk/vdisk", "rb+");
	if (!fp) fp = fopen("../disk/vdisk", "wb+");

	return fp;
}

int writeBlock(void *input, int size, int num, int blockNum){

	FILE* fp = openVDisk();
	if (!fp){
		printf("Error Opening vdisk");
		return -2;
	}

	if (size*num > BLOCK_SIZE || sizeof(input) > BLOCK_SIZE)
		{
		fclose(fp);
		return -1;
	}

	fseek(fp, blockNum*BLOCK_SIZE, SEEK_SET);

	if (fwrite(input, size, num, fp)!= num){
		fclose(fp);
		return -2;
	}
	fclose(fp);
	return 1;
}

int readBlock(int blockNum, void* p){
	if (blockNum >= MAX_BLOCKS) return -1;

	FILE* fp;
	fp = openVDisk();
	if (!fp){
		printf("Error Opening vdisk");
		return -2;
	}
	fseek(fp, blockNum*BLOCK_SIZE, SEEK_SET);
	fread(p, BLOCK_SIZE, 1, fp);
	fclose(fp);
	return 1;
}

#ifndef TESTING

int main(int argc, char *argv[]){

	int data[512];
	memset(data, 0, 512);
	char chard[512];
	memset(chard, 0, 512);
	chard[0] = 'a';
	chard[4] = 'd';


	printf("%d\n", writeBlock(data, sizeof(int), 128, 0));
	writeBlock(chard, 1, 5, 0);

	data[0] = 'A';
	data[1] = 'B';
	data[2] = 'C';

	writeBlock(data, 4, 5, 1);

	//int data_out[BLOCK_SIZE/4];
	//readBlock(0, data_out);
	//printf("%d%d\n", data_out[0], data_out[3]);




}
#endif
