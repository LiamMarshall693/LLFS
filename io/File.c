#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../disk/DiskController.h"

#include "File.h"

struct inode{
	int id;
	int filesize;
	int flags;
	short int blocknumbers[10];
	short int sing_ind;
	short int doub_ind;

};

struct directory{
	int count;
	short int blocknum;
	struct inode inode;
	short int inodes[16];
	char filename[16][31];

};

/*

typedef struct {
	char* buffer;
	int blocks;
	int inodes;
	int size;
	char* path;
	short int path_len;
	short int* updates;

} LLFS_Interface;
*/

struct directory newDirectory(short int logHead);
short int getInodeBlock(short int id);
short int nextBlock(int loghead);
short int nextInode();

/*
 * Disk Structure
 * Superblock:
 * Bytes 0-3: Magic number
 * Bytes 4-7: Number of blocks on disk
 * Bytes 8-11: Number of inodes available
 * Bytes 12-13: Segment size
 * Bytes 14-15: Current head of log
 * Bytes 16-47: Free Inode vectors
 *
 * Block #1:
 * Free block vectors
 *
 * Block #2:
 * Inode Map
 *
 * Blocks 10-4095:
 * Data blocks
 */

int LLFS_Init(LLFS_Interface *LLFS_i){

	printf("start init\n");
	//init superblock
	int mgc = MAGIC_NUM;
	int max_blocks = MAX_BLOCKS;
	int max_inodes = MAX_INODES;
	int int_buf[MAX_BLOCKS/4];
	char char_buf[BLOCK_SIZE];

	memset(int_buf, 0, BLOCK_SIZE);
	memcpy(&int_buf, &mgc, 4);
	memcpy(&int_buf[1], &max_blocks, 4);
	memcpy(&int_buf[2], &max_inodes, 4);
	//printf("superblock written\n");

	//free inode vector;
	memset(char_buf, 255, BLOCK_SIZE);
	memcpy(&int_buf[4], char_buf, 32);
	memset(char_buf, 127, 1);
	memcpy(&int_buf[4], char_buf, 1);


	writeBlock(int_buf, 512, 1, 0);
	//printf("free inode block written\n");


	//init free block vector
	//initially, first 10 are cleared, rest are set
	memset(char_buf, 255, BLOCK_SIZE);
	char_buf[0] = 0;
	char_buf[1] = 63;
	writeBlock(char_buf, 512, 1, 1);

	//inode map block


	//zero out resk of disk
	memset(int_buf, 0, BLOCK_SIZE);
	for (int i = 2; i<4096; i++){
		writeBlock(int_buf, BLOCK_SIZE, 1, i);
	}
	//printf("rest of disk cleared \n");

	//create root directory and add to superblock
	newDirectory(10);
	//printf("new dir complete\n");
	short int seg_size = 10;
	short int logHead = 12; //10 metadata blocks, plus root inode and directory blocks
	short int block[256];
	readBlock(0, block);
	memcpy(&block[6], &seg_size, 2);
	memcpy(&block[7], &logHead, 2);
	writeBlock(block, 512, 1, 0);

	getInodeBlock(1);


	printf("root etc added\n");
	//char* rootpath = "/";
	//memcpy(LLFS_i->path, rootpath, 1);
	printf("memcpy\n");
	LLFS_i->path_len = 1;
	printf("path_len added\n");
	LLFS_i->inodes++;
	printf("Init complete\n");
	return 1;
}

int printBits(char *input, int bytes){
  for(int i = 0; i<bytes; i++){
	  for (int j = 0; j < 8; j++) {
		  printf("%d", !!((input[i] << j) & 0x80));
	  }
	  printf(" ");
  }
  printf("\n");

  return 0;
}

int setBVector(short int blockNum){
	char block[BLOCK_SIZE];
	readBlock(1, block);

	int off = blockNum/8;
	int r = blockNum%8;

	if (r == 0) block[off] = block[off] | 128;
	if (r == 1) block[off] = block[off] | 64;
	if (r == 2) block[off] = block[off] | 32;
	if (r == 3) block[off] = block[off] | 16;
	if (r == 4) block[off] = block[off] | 8;
	if (r == 5) block[off] = block[off] | 4;
	if (r == 6) block[off] = block[off] | 2;
	if (r == 7) block[off] = block[off] | 1;

	writeBlock(block, BLOCK_SIZE, 1, 1);
	return 1;
}

int clrBVector(short int blockNum){
	char block[BLOCK_SIZE];
	readBlock(1, block);

	int off = blockNum/8;
	int r = blockNum%8;

	if (r == 0) block[off] = block[off] & 127;
	if (r == 1) block[off] = block[off] & 191;
	if (r == 2) block[off] = block[off] & 223;
	if (r == 3) block[off] = block[off] & 239;
	if (r == 4) block[off] = block[off] & 247;
	if (r == 5) block[off] = block[off] & 251;
	if (r == 6) block[off] = block[off] & 253;
	if (r == 7) block[off] = block[off] & 254;

	writeBlock(block, BLOCK_SIZE, 1, 1);
	return 1;
}

int setIVector(short int inodeNum){
	if (inodeNum > 255 || inodeNum < 1) return -1;
	char block[BLOCK_SIZE];
	readBlock(0, block);

	int off = inodeNum/8 + 16;
	int r = inodeNum%8;
	printf("setIVector %d off = %d r = %d\n", inodeNum, off, r);

	if (r == 0) block[off] = block[off] | 128;
	if (r == 1) block[off] = block[off] | 64;
	if (r == 2) block[off] = block[off] | 32;
	if (r == 3) block[off] = block[off] | 16;
	if (r == 4) block[off] = block[off] | 8;
	if (r == 5) block[off] = block[off] | 4;
	if (r == 6) block[off] = block[off] | 2;
	if (r == 7) block[off] = block[off] | 1;

	writeBlock(block, BLOCK_SIZE, 1, 0);
	return 1;
}

int clrIVector(short int inodeNum){
	if (inodeNum > 255 || inodeNum < 1) return -1;
	char block[BLOCK_SIZE];
	readBlock(0, block);

	int off = inodeNum/8 + 16;
	int r = inodeNum%8;

	if (r == 0) block[off] = block[off] & 127;
	if (r == 1) block[off] = block[off] & 191;
	if (r == 2) block[off] = block[off] & 223;
	if (r == 3) block[off] = block[off] & 239;
	if (r == 4) block[off] = block[off] & 247;
	if (r == 5) block[off] = block[off] & 251;
	if (r == 6) block[off] = block[off] & 253;
	if (r == 7) block[off] = block[off] & 254;

	writeBlock(block, BLOCK_SIZE, 1, 0);


	return 1;
}

struct inode writeInode(int size, short int logHead, int flag){

	struct inode in;
	short int id = nextInode();
	in.id = id;
	if (id == -1) return in;
	clrIVector(id);
	in.flags = flag;
	in.filesize = size;
	int blocks = size/BLOCK_SIZE;
	short int sing_ind = 0;
	if (size%BLOCK_SIZE > 0 || flag ==1 ) blocks++;
	memset(in.blocknumbers, 0, 20);
	//printf("init inode complete\n");

	for (int i = 0; i < blocks && i < 10; i++){
		in.blocknumbers[i] = nextBlock(logHead);
		if (in.blocknumbers[i] == -1) in.id = -1;
		//printf("%d\n", in.blocknumbers[i]);
		clrBVector(in.blocknumbers[i]);
	}

	//single indirect
	if (blocks > 10){
		sing_ind = nextBlock(logHead);
		if (sing_ind == -1) in.id = -1;
		in.sing_ind = sing_ind;
		clrBVector(sing_ind);
		short int buf[BLOCK_SIZE/2];
		memset(buf, 0, BLOCK_SIZE);

		for (int i = 10; i < blocks && i < 266; i++){
			buf[i-10] = nextBlock(logHead);
			if (buf[i-10] == -1) in.id = -1;
			clrBVector(buf[i-10]);
		}
		writeBlock(buf, 512, 1, sing_ind);

	}
	//printf("blocks assigned to inode\n");

	//double indirect block INCOMPLETE
	/*
	if (blocks > 266){
		short int doub_ind = nextBlock();
		in.doub_ind = doub_ind;
		clrBVector(doub_ind);
		short int buf[BLOCK_SIZE];
		memset(buf, 0, BLOCK_SIZE);

		for (int i = 266; i < 256*256 ; i++){
			short int sing_ind = nextBlock();
			buf[i] = nextBlock();
			clrBVector(buf[i]);
		}
		blockWrite(doub_ind, buf);
	}
	*/
	if (in.id == -1) return in;

	if (flag == 1) size = 0;
	short int blockNum = nextBlock(logHead);
	char block[512];
	memset(block, 0, 512);
	memcpy(&block[0], &size, 4);
	memcpy(&block[4], &in.flags, 4);
	memcpy(&block[8], in.blocknumbers, 20);
	memcpy(&block[28], &sing_ind, 2);
	writeBlock(block, 512, 1, blockNum);
	clrBVector(blockNum);

	memset(block, 0, BLOCK_SIZE);
	readBlock(2, block);
	memcpy(&block[id*2], &blockNum, 2);
	writeBlock(block, 512, 1, 2);
	//printf("write inode id = %d complete with blocknumber[0] = %d written to block %d\n", id, in.blocknumbers[0], blockNum);

	return in;
}

short int nextBlock(int loghead){
	char block[512];
	readBlock(1, block);

	int start = loghead/8;

	for (short int i = start; i < 512;i++){
		if (block[i] == 0) continue;
		if ((block[i] & 128) ==128) return (i)*8;
		if ((block[i] & 64) == 64) return (i)*8 +1;
		if ((block[i] & 32) == 32) return (i)*8 +2;
		if ((block[i] & 16) == 16) return (i)*8 +3;
		if ((block[i] & 8) == 8) return (i)*8 +4;
		if ((block[i] & 4) == 4) return (i)*8 +5;
		if ((block[i] & 2) == 2) return (i)*8 +6;
		if ((block[i] & 1) == 1) return (i)*8 +7;

	}
	printf("ERROR: no new free blocks in block vector");
	return -1;
}

short int nextInode(){

	char block[512];
	readBlock(0, block);

	for (short int i = 16; i < 48;i++){
		if (block[i] == 0) continue;
		if ((block[i] & 128) ==128) return (i-16)*8;
		if ((block[i] & 64) == 64) return (i-16)*8 +1;
		if ((block[i] & 32) == 32) return (i-16)*8 +2;
		if ((block[i] & 16) == 16) return (i-16)*8 +3;
		if ((block[i] & 8) == 8) return (i-16)*8 +4;
		if ((block[i] & 4) == 4) return (i-16)*8 +5;
		if ((block[i] & 2) == 2) return (i-16)*8 +6;
		if ((block[i] & 1) == 1) return (i-16)*8 +7;

	}
	printf("ERROR: no new free inodes in inode vector");
	return -1;
}

struct directory newDirectory(short int logHead){
	struct inode inode = writeInode(0, logHead, 1);
	if (inode.id == -1) printf("Error assigning Inode");
	struct directory dir;
	dir.inode = inode;
	dir.blocknum = inode.blocknumbers[0];
	return dir;
}

short int getInodeBlock(short int id){
	char block[512];
	readBlock(2, block);
	short int blockNum;
	memcpy(&blockNum, &block[id*2], 2);
	//printf("Get Inode Block id %d return %d\n", id, blockNum);
	return blockNum;
}

short int setInodeBlock(short int id, short int blocknum){
	char block[512];
	readBlock(2, block);
	memcpy(&block[id*2], &blocknum, 2);
	writeBlock(block, 512, 0, 2);
	return 1;
}

struct inode readPath(char *path, short int path_len){
	//char str[path_len];
	//memcpy(str, path, pathlen);
	printf("Start readPath w/ str = %s\n", path);

	short int root_Inode_BlockNum = getInodeBlock(1);
	char dir_inode_Block[BLOCK_SIZE];
	readBlock(root_Inode_BlockNum, dir_inode_Block);
	int dir_size;
	int dir_flags;
	short int dir_id = 1;
	short int dir_blockNumbers[10];
	short int dir_sing_ind;
	memcpy(&dir_size, dir_inode_Block, 4);
	memcpy(&dir_flags, &dir_inode_Block[4], 4);
	memcpy(dir_blockNumbers, &dir_inode_Block[8], 20);
	memcpy(&dir_sing_ind, &dir_inode_Block[28], 2);
	//printf("dir data set, root_inode_blocknum = %d\n", root_Inode_BlockNum);

	//printBits(&dir_inode_Block[8], 20);
	struct inode in;

	struct inode err;
	err.id = -1;

	if (path_len <=1){
		in.filesize = dir_size;
		in.flags = dir_flags;
		memcpy(in.blocknumbers, dir_blockNumbers, 20);
		in.sing_ind = dir_sing_ind;
		in.id = 1;
		//printf("dir_Inode filesize = %d, in.blocknumbers = %d\n", dir_size, in.blocknumbers[0]);
		//printf("Returning root dir\n");
		return in;
	}

	char token[path_len];
	memset(token, 0, path_len);
	int i = 0;
	int c = 0;


	if (path[0] == '/') i++;

	for (; i < path_len; i++){
		if (i != path_len && path[i] != '/'){
			token[c] = path[i];
			c++;
		}
		else{
			int found = 0;
			//printf("searching for token %s\n", token);
			for (int j = 0; j<10; j++){ // loop through data blocks pointed to by dir_inode
				char dir_block[BLOCK_SIZE];
				readBlock(dir_blockNumbers[j], dir_block);


				char filename[31];
				memset(filename, 0, 31);
				for (int k = 0; k< 16; k++){ // loop through files in dir data block

					memcpy(filename, &dir_block[k*32+1], 31);
					int match = 1;
					//printf("Checking token agsinst str: %s \n", filename);
					for (int l = 0; l < path_len && l < 31; l++){
						//printf("%c.%c ", filename[l], token[l]);
						if (filename[l] != token[l]) match = 0;
					}
					//printf("\n");

					if (match == 1){//found the next directory
						//printf("match found\n");
						found = 1;
						short int file_inode_id;
						memset(&file_inode_id, 0 , 2);
						memcpy(&file_inode_id, &dir_block[k*32], 1); //this needs to be 1 byte even though short int, due to dir format specifications

						readBlock(getInodeBlock(file_inode_id), dir_inode_Block);

						in.id = file_inode_id;
						dir_id = file_inode_id;

						memcpy(&dir_size, dir_inode_Block, 4);
						memcpy(&dir_flags, &dir_inode_Block[4], 4);
						memcpy(dir_blockNumbers, &dir_inode_Block[8], 20);
						memcpy(&dir_sing_ind, &dir_inode_Block[28], 2);

						if (dir_flags != 1){
							printf("Error: file \"%s\" in path is not of type directory. (file_inode_id= %d), dir_flags = %d, blocknum = %d\n", token, file_inode_id, dir_flags, getInodeBlock(file_inode_id));
							return err;
						}
						k = 100;
						j = 100;
					}
				}
				if (found == 0){
					printf("Error: file \"%s\" not found in parent directory(id = %d)\n", token, dir_id);
					return err;
				}
			}


			memset(token, 0 , path_len);
			c = 0;
		}
	}
	in.filesize = dir_size;
	in.flags = dir_flags;
	memcpy(in.blocknumbers, dir_blockNumbers, 20);
	in.sing_ind = dir_sing_ind;
	//printf("dir_Inode filesize = %d\n", dir_size);
	//printf("finish readPath()\n");
	return in;
}

short int getRoot(){
	short int block[512];
	readBlock(0, block);
	return block[6];
}

/*
 * removeFile takes an inode struct as an argument, and clears all the memory pointed to by
 * that inode.  The Inode map and both inode and data block vectors are cleared.
 * If the inode is a directory, it is called recursively on all inode within the
 * directory.  Note that the data block of the parent directory is not modifed, changes to the
 * parent directory must be done by the method which calls removeFile.  If the calling method is updating
 * rather than deleting, note that the inode vector and inode map have been cleared by this method
 */

int removeFile(struct inode in){

	//printf("begin remove file id = %d\n", in.id);
	//check if in buffer
	int blocks = in.filesize/BLOCK_SIZE;
	if (in.filesize%BLOCK_SIZE >0) blocks++;

	if(in.flags == 1){

		int j = in.filesize;
		for (int i = 0; i < (in.filesize/16)+1; i++){
			char block[BLOCK_SIZE];
			memset(block, 0, BLOCK_SIZE);
			readBlock(in.blocknumbers[i%16], block);
			for (int k = 0; k<16 && j > 0; k++){
				j--;
				short int file_inode_id;
				memset(&file_inode_id, 0, 2);
				memcpy(&file_inode_id, &block[32*k], 1);

				char inode_block[BLOCK_SIZE];
				memset(inode_block, 0, BLOCK_SIZE);
				readBlock(getInodeBlock(file_inode_id), inode_block);

				struct inode file_in;
				file_in.id = file_inode_id;
				memcpy(&file_in.filesize, inode_block, 4);
				memcpy(&file_in.flags, &inode_block[4], 4);
				memcpy(file_in.blocknumbers, &inode_block[8], 20);
				memcpy(&file_in.sing_ind, &inode_block[28], 2);

				//printf("beginning recusrion\n");
				removeFile(file_in);
			}
		}

		blocks = (in.filesize/16)+1;
	}

	char buffer[BLOCK_SIZE];
	memset(buffer, 0 , BLOCK_SIZE);

	for (int i = 0; i < blocks; i++){
		short int blocknum = in.blocknumbers[i];
		//printf("clearing block %d for inode %d\n", blocknum, in.id);
		writeBlock(buffer, BLOCK_SIZE, 1, blocknum);
		setBVector(blocknum);
	}

	writeBlock(buffer, BLOCK_SIZE, 1, getInodeBlock(in.id));
	setIVector(in.id);
	printf("set inode block %d for id %d\n", getInodeBlock(in.id), in.id);
	memset(buffer, 0 , BLOCK_SIZE);
	writeBlock(buffer, BLOCK_SIZE, 1, getInodeBlock(in.id));
	memset(buffer, 0 , BLOCK_SIZE);
	readBlock(2, buffer);
	memset(&buffer[in.id*2], 0, 2);


	return 1;
}

/*
 * Takes as arguments an inode for a directory, and an index of a file which has just been removed.
 * The last entry of the directory is copied into the slot occupied by the removed file, and the last
 * file's previous slot is wiped clear.  Note that this does not preserve the original ordering of the
 * files in the directory, and does not adjust the directory filesize in the directory's inode
 */

int replaceEntry(struct inode dir, int index){
	printf("Start replace with filesize of parent = %d\n", dir.filesize);
	char bufferfrom[BLOCK_SIZE];
	memset(bufferfrom, 0, BLOCK_SIZE);
	readBlock(dir.blocknumbers[(dir.filesize-1)/16], bufferfrom);

	printf("from = %d to = %d\n", (index%16)*32, ((dir.filesize-1)%16)*32);
	char bufferto[BLOCK_SIZE];
	memset(bufferto, 0, BLOCK_SIZE);
	readBlock(dir.blocknumbers[index/16], bufferto);

	memcpy(&bufferto[(index%16)*32], &bufferfrom[((dir.filesize-1)%16)*32], 32);
	memset(&bufferfrom[((dir.filesize-1)%16)*32], 0, 32);
	//printf("from = %d to = %d, write %d then %d\n", dir.filesize-1%16, index%16, (dir.filesize-1)/16, index/16);

	writeBlock(bufferto, 512, 1, dir.blocknumbers[index/16]);
	writeBlock(bufferfrom, 512, 1, dir.blocknumbers[(dir.filesize-1)/16]);

	return 1;
}

/*
 * Takes a file name argument, and checks whether or not that file, (at the location
 * of LLFS_i's current path) is present in LLFS_i's buffer.  Used by methods such as
 * LLFS_rm and LLFS_modify to see whether the requested file has been created but not
 * yet written to disk from the buffer.
 */

int checkInBuffer(LLFS_Interface *LLFS_i, char* name, short int name_len){
	int bufsize = LLFS_i->size;
	int count = 0;
	while (count < bufsize){
		int size;
		short int path_len;
		short int buf_name_len;
		memcpy(&size, &(LLFS_i->buffer[count]), 4);
		memcpy(&path_len, LLFS_i->buffer + count+4, 2);
		memcpy(&buf_name_len, &(LLFS_i->buffer[count+6]), 2);
		count += 8;
		char buf_path[path_len];
		char buf_name[buf_name_len];
		memcpy(&buf_path, &LLFS_i->buffer[count], path_len);
		memcpy(&buf_name, &(LLFS_i->buffer[count+path_len]), name_len);

		int match = 1;
		if (name_len != buf_name_len) match = 0;
		for (int l = 0; l < name_len && l < buf_name_len; l++){
			//printf("%c.%c ", filename[l], token[l]);
			if (buf_name[l] != name[l]) match = 0;
		}
		for (int l = 0; l < path_len && l < LLFS_i->path_len; l++){
			//printf("%c.%c ", filename[l], token[l]);
			if (buf_path[l] != LLFS_i->path[l]) match = 0;
		}
		if (match == 1){
			return count - 8;
		}
		count += size + buf_name_len + path_len;

	}
	return -1;
}

int writeBuffer(LLFS_Interface *LLFS_i, short int logHead){
	printf("Start writeBuffer\n");
	int count = 0;
	int count_inodes = 0;
	while (count < LLFS_i->size){
		//printf("writeBuffer, count = %i\n", count);
		int size;
		short int path_len;
		short int name_len;
		memcpy(&size, &(LLFS_i->buffer[count]), 4);
		memcpy(&path_len, LLFS_i->buffer + count+4, 2);
		memcpy(&name_len, &(LLFS_i->buffer[count+6]), 2);
		count += 8;
		//printf("cpy lens complete, path_len = %hu\n", path_len);

		char path[path_len];
		char name[name_len];
		memcpy(&path, &(LLFS_i->buffer[count]), path_len);
		count += path_len;

		//printf("path written: %s, path_len: %i\n", path, path_len);
		memcpy(&name, &(LLFS_i->buffer[count]), name_len);
		count += name_len;

		//printf("struct dir\n");
		struct directory dir;
		struct inode in;
		if (size == -1){
			dir = newDirectory(logHead);
			in = dir.inode;
		}else{
			in = writeInode(size, logHead, 0);
		}

		//writeInode will have assigned a new Inode to the file, regardless of whether the buffer
		//stores an existing file or a modification
		//If modification, we need to set the old inode to point to the new file data, and clear
		//out the new inode again
		if (LLFS_i->updates[count_inodes] != 0){
			short int id = LLFS_i->updates[count_inodes];
			setInodeBlock(id, getInodeBlock(in.id));
			setIVector(in.id);
			setInodeBlock(in.id, 0);
		}

		//if not modification, add new entry to parent directory
		else{

			//printf("in.blocknumbers[0] = %d\n", in.blocknumbers[0]);
			//printf("ParentDir\n");
			struct inode parentDir = readPath(path, path_len);
			if (parentDir.id == -1){
				printf("Reading path failed, file %s not written from buffer", name);
				return -1;
			}

			//printf("in.blocknumbers[0] = %d\n", in.blocknumbers[0]);
			//printf("after readPath, parentDir size = %d\n", parentDir.filesize);

			short int dir_block_num = parentDir.blocknumbers[parentDir.filesize/16];
			//printf("parentDir.blocknumbers = %d\n", dir_block_num);
			char inode_block[BLOCK_SIZE];
			char dir_block[BLOCK_SIZE];

			readBlock(getInodeBlock(parentDir.id), inode_block);

			readBlock(dir_block_num, dir_block);

			//printf("Before writes to paren, (32*parentDir.filesize/16) = %d\n", parentDir.filesize);
			memset(&dir_block[32*(parentDir.filesize%16)], 0, 32);
			memcpy(&dir_block[32*(parentDir.filesize%16)], &in.id, 1);
			memcpy(&dir_block[(32*(parentDir.filesize%16)) +1], &name, name_len);
			writeBlock(dir_block, 512, 1, dir_block_num);

			parentDir.filesize++;
			memcpy(inode_block, &parentDir.filesize, 4);
			writeBlock(inode_block, 512, 1, getInodeBlock(parentDir.id));

		}
		//printf("chunks \n");
		int chunks = size/BLOCK_SIZE;
		int r = size%BLOCK_SIZE;
		if (r >0) chunks++;
		int amount_remaining = size;

		//printf("Before data write\n");

		for (int i = 0;i < chunks; i++){

			int chunk_size = BLOCK_SIZE;
			if (amount_remaining < BLOCK_SIZE) chunk_size = amount_remaining;
			char block[BLOCK_SIZE];
			memset(block, 0, BLOCK_SIZE);
			memcpy(block, &(LLFS_i->buffer[count]), chunk_size);

			amount_remaining -= BLOCK_SIZE;
			//printf("in data write, count = %d\n", count);
			writeBlock(block, 512, 1, in.blocknumbers[i]);
			count += size;
		}
		count_inodes++;
		//printf("After data write, size = %d\n", LLFS_i->size);
	}
	//printf("About to return\n");
	char superblock[BLOCK_SIZE];
	readBlock(0, superblock);
	logHead += LLFS_i->blocks;
	memcpy(&superblock[14], &logHead, 2);
	writeBlock(superblock, 512, 1, 0);

	memset(LLFS_i->updates, 0, LLFS_i->inodes*2);
	LLFS_i->buffer = malloc(0);
	LLFS_i->size = 0;
	LLFS_i->blocks = 0;
	LLFS_i->inodes = 0;
	return 1;
}

LLFS_Interface* OpenLLFS(){
	printf("OPenLLFS start\n");

	LLFS_Interface *n = (LLFS_Interface*) malloc(sizeof(LLFS_Interface));
	n->buffer = (char*) malloc(1);
	n->path = "/";
	n->path_len = 1;
	//printf("Mallocs complete\n");
	n->blocks = 0;
	n->size = 0;
	n->inodes = 0;

	char superblock[BLOCK_SIZE];
	readBlock(0, superblock);
	short int seg_size;
	memcpy(&seg_size, &superblock[12], 2);
	n->updates = malloc(2 * (seg_size+1));
	if (!n->updates) printf("Error Mallocing updates in OpenLLFS\n");
	memset(n->updates, 0, (seg_size+1)*2);

	//printf("OpenLLFS complete\n");
	return n;
}

int LLFS_new(LLFS_Interface *LLFS_i, char* name, char* data, int size){
	printf("new start\n");


	char superblock[BLOCK_SIZE];
	readBlock(0, superblock);
	short int seg_size;
	short int logHead;
	memcpy(&seg_size, &superblock[12], 2);
	memcpy(&logHead, &superblock[14], 2);


	short int path_len = LLFS_i->path_len;
	short int name_len = strlen(name);

	//printf("in new, path_len = %i, name_len = %i\n", path_len, name_len);


	int new_size = LLFS_i->size + size + 8 + path_len + name_len;

	LLFS_i->buffer = realloc(LLFS_i->buffer, new_size);
	char* buffer = LLFS_i->buffer;
	memset(&(LLFS_i->buffer)[LLFS_i->size], 0, new_size - LLFS_i->size);
	printf("Realloc complete, LLFS_i->path = %s\n", LLFS_i->path);

	memcpy(buffer + LLFS_i->size, &size, 4);
	memcpy(&buffer[LLFS_i->size+4], &path_len, 2);
	memcpy(&buffer[LLFS_i->size+6], &name_len, 2);
	memcpy(&buffer[LLFS_i->size+8], LLFS_i->path, path_len);
	memcpy(&buffer[LLFS_i->size+8+path_len], name, name_len);
	memcpy(&buffer[LLFS_i->size+8+path_len+name_len], data, size);
	//printf("Bufferwrite complete, name = %i\n", res);

	memcpy(&LLFS_i->size, &new_size, 4);


	int blocks = LLFS_i->blocks + 1 + (size/BLOCK_SIZE);
	if (size%BLOCK_SIZE > 0) blocks++;
	memcpy(&(LLFS_i->blocks), &blocks, 4);
	int inodes = LLFS_i->inodes + 1;
	memcpy(&(LLFS_i->inodes), &inodes, 4);


	//test:
	printf("starting writeBuffer\n");

	//printf("LLFS_i->path = %s\n", LLFS_i->path);


	if(LLFS_i->blocks > seg_size) writeBuffer(LLFS_i, logHead);
	//printf("Done writing buffer\n");




	return 1;
}

int LLFS_modify(LLFS_Interface *LLFS_i, char* name, char* data, int size){

	int buf_index = checkInBuffer(LLFS_i, name, strlen(name));

	if (buf_index == -1){ //if file is not in buffer, simply add file with update tag
		short int file_inode_id;
		struct inode dir = readPath(LLFS_i->path, LLFS_i->path_len);
		char dir_block[BLOCK_SIZE];
		char filename[31];
		memset(filename, 0, 31);
		int found = 0;

		//find the file in the parent directory, and the associated inode id
		for (int j = 0; j<10 && j<=dir.filesize; j++){ // loop through data blocks pointed to by dir
			memset(dir_block, 0 , BLOCK_SIZE);
			readBlock(dir.blocknumbers[j], dir_block);
			for (int k = 0; k< 16; k++){ // loop through files in dir data block

				memcpy(filename, &dir_block[k*32+1], 31);
				int match = 1;
				printf("Checking token agsinst str: %s\n", filename);
				for (int l = 0; l < strlen(name) && l < 31; l++){
					printf("%c.%c ", filename[l], name[l]);
					if (filename[l] != name[l]) match = 0;
				}
				printf("\n");

				if (match == 1){//found the next directory
					printf("match found\n");
					found = 1;
					memset(&file_inode_id, 0 , 2);
					memcpy(&file_inode_id, &dir_block[k*32], 1); //this needs to be 1 byte even though short int, due to dir format specifications

					j = 100;
					k = 100;
				}
			}
		}
		if (found == 0){
			printf("Error: file \"%s\" not found in parent directory(id = %d)\n", name, dir.id);
			return -1;
		}
		//add to buffer with inode id as update tag
		LLFS_new(LLFS_i, name, data, size);
		LLFS_i->updates[LLFS_i->inodes-1] = file_inode_id;
	}
	else{
		int prev_size;
		short int path_len;
		short int name_len;
		memcpy(&prev_size, &LLFS_i->buffer[buf_index], 4);
		memcpy(&path_len, &LLFS_i->buffer[buf_index +4], 2);
		memcpy(&name_len, &LLFS_i->buffer[buf_index+6], 2);
		if (size == -1){
			printf("Cannot use LLFS_modify on directories\n");
			return -1;
		}
		int header_len = 8 + path_len + name_len;

		memcpy(&LLFS_i->buffer[buf_index + header_len + size], &LLFS_i->buffer[buf_index+header_len+prev_size], LLFS_i->size - buf_index - header_len - prev_size);
		memcpy(&LLFS_i->buffer[buf_index + header_len], data, size);

		LLFS_i->size -= prev_size - size;
		LLFS_i->blocks -= prev_size/BLOCK_SIZE;
		if (prev_size%BLOCK_SIZE >0) LLFS_i->blocks--;
		LLFS_i->blocks += size/BLOCK_SIZE;
		if (size%BLOCK_SIZE >0) LLFS_i->blocks--;
	}

	return 1;
}

int LLFS_flush(LLFS_Interface *LLFS_i){
	char superblock[BLOCK_SIZE];
	readBlock(0, superblock);
	short int logHead;
	memcpy(&logHead, &superblock[14], 2);
	writeBuffer(LLFS_i, logHead);
	return 1;
}

int LLFS_mkdir(LLFS_Interface *LLFS_i, char* name){
	printf("Start mkdir\n");

	char superblock[BLOCK_SIZE];
	readBlock(0, superblock);
	short int seg_size;
	short int logHead;
	memcpy(&seg_size, &superblock[12], 2);
	memcpy(&logHead, &superblock[14], 2);

	short int path_len = strlen(LLFS_i->path);
	short int name_len = strlen(name);

	int new_size = LLFS_i->size + 8 + path_len + name_len;

	LLFS_i->buffer = realloc(LLFS_i->buffer, new_size);
	char* buffer = LLFS_i->buffer;
	memset(&(LLFS_i->buffer)[LLFS_i->size], 0, new_size - LLFS_i->size);

	int size = -1;
	memcpy(buffer + LLFS_i->size, &size, 4);
	memcpy(&buffer[LLFS_i->size+4], &path_len, 2);
	memcpy(&buffer[LLFS_i->size+6], &name_len, 2);
	memcpy(&buffer[LLFS_i->size+8], LLFS_i->path, path_len);
	memcpy(&buffer[LLFS_i->size+8+path_len], name, name_len);

	memcpy(&LLFS_i->size, &new_size, 4);

	int blocks = LLFS_i->blocks + 1;
	memcpy(&(LLFS_i->blocks), &blocks, 4);
	int inodes = LLFS_i->inodes + 1;
	memcpy(&(LLFS_i->inodes), &inodes, 4);

	writeBuffer(LLFS_i, logHead);

	return 1;

}

int LLFS_cd(LLFS_Interface *LLFS_i, char* path){
	//printf("Start cd\n");
	struct inode in = readPath(path, (short int) strlen(path));

	if (in.id == -1){
		printf("Error occured in readPath(), cd failed\n");
		return -1;
	}

	//printf("in.id = %d\n", in.id);

	LLFS_i->path = path;
	LLFS_i->path_len = (short int) strlen(path);
	return 1;
}

int LLFS_ls(LLFS_Interface *LLFS_i){
	//printf("Start ls\n");
	struct inode in = readPath(LLFS_i->path, LLFS_i->path_len);
	if (in.id == -1){
		printf("Error occured in readPath(), ls failed\n");
		return -1;
	}

	if( in.filesize/16+1 == 0){
		printf("Directory is empty\n");
		return 1;
	}

	//printf("continuing ls\n");
	for(int i = 0; i< (in.filesize/16)+1; i++){
		char block[BLOCK_SIZE];
		memset(block, 0, BLOCK_SIZE);
		readBlock(in.blocknumbers[i], block);

		//printf("read block %d\n", in.blocknumbers[i]);
		for (int j = 0; j< 16; j++){
			char filename[31];
			memcpy(filename, &block[32*j+1], 31);
			char inode_id;
			memcpy(&inode_id, &block[32*j], 1);
			if (inode_id != 0){
				printf("%s ", filename);
				if (j==8) printf("/n");
			}
		}
		printf("\n");
	}
	return 1;
}

int LLFS_rm(LLFS_Interface *LLFS_i, char* name){
	printf("start rm\n");

	int buf_index = checkInBuffer(LLFS_i, name, strlen(name));


	//file is in buffer, not yet written to disk: remove from buffer
	if (buf_index != -1){
		int size;
		short int path_len;
		short int name_len;
		memcpy(&size, &LLFS_i->buffer[buf_index], 4);
		memcpy(&path_len, &LLFS_i->buffer[buf_index +4], 2);
		memcpy(&name_len, &LLFS_i->buffer[buf_index+6], 2);
		if (size == -1) size = 0;
		int buf_seg_size = 8 + path_len + name_len + size;

		memcpy(&LLFS_i->buffer[buf_index], &LLFS_i->buffer[buf_index+buf_seg_size], LLFS_i->size - buf_index - buf_seg_size);
		LLFS_i->size -= buf_seg_size;
		LLFS_i->inodes--;
		LLFS_i->blocks -= size/BLOCK_SIZE +2;
		if (size%BLOCK_SIZE >0) LLFS_i->blocks--;

		return 1;
	}
	//otherwise, find on disk and clear

	//open working directory
	struct inode dir = readPath(LLFS_i->path, LLFS_i->path_len);

	if (dir.id == -1){
		printf("Error occured in readPath(), rm failed\n");
		return -1;
	}

	printf("dir.id = %d\n", getInodeBlock(dir.id));
	//read working directory to find file to remove
	char dir_block[BLOCK_SIZE];

	char filename[31];
	memset(filename, 0, 31);
	int index;

	struct inode file_in;
	int found = 0;
	for (int j = 0; j<10; j++){ // loop through data blocks pointed to by dir
		memset(dir_block, 0 , BLOCK_SIZE);
		readBlock(dir.blocknumbers[j], dir_block);
		for (int k = 0; k< 16; k++){ // loop through files in dir data block

			memcpy(filename, &dir_block[k*32+1], 31);
			int match = 1;
			printf("Checking token agsinst str: %s\n", filename);
			for (int l = 0; l < strlen(name) && l < 31; l++){
				printf("%c.%c ", filename[l], name[l]);
				if (filename[l] != name[l]) match = 0;
			}
			printf("\n");

			if (match == 1){//found the next directory
				//printf("match found\n");
				found = 1;

				short int file_inode_id;
				index = (j*16)+k;
				memset(&file_inode_id, 0 , 2);
				memcpy(&file_inode_id, &dir_block[k*32], 1); //this needs to be 1 byte even though short int, due to dir format specifications

				file_in.id = file_inode_id;
				char file_inode_block[BLOCK_SIZE];
				memset(file_inode_block, 0 , BLOCK_SIZE);
				readBlock(getInodeBlock(file_inode_id), file_inode_block);

				memcpy(&file_in.filesize, file_inode_block, 4);
				memcpy(&file_in.flags, &file_inode_block[4], 4);
				memcpy(file_in.blocknumbers, &file_inode_block[8], 20);
				memcpy(&file_in.sing_ind, &file_inode_block[28], 2);

				j = 100;
				k = 100;
			}
		}
	}
	if (found == 0){
		printf("Error: file \"%s\" not found in parent directory(id = %d)\n", name, dir.id);
		return -1;
	}
	printf("LLFS_rm finished, starting removeFIle(), index = %d\n", index);
	removeFile(file_in);
	replaceEntry(dir, index);
	dir.filesize--;
	char inode_block[BLOCK_SIZE];
	memset(inode_block, 0, BLOCK_SIZE);
	readBlock(getInodeBlock(dir.id), inode_block);
	memcpy(inode_block, &dir.filesize, 4);
	writeBlock(inode_block, 512, 1, getInodeBlock(dir.id));
	return 1;

}

#ifdef TESTING
int main(int argc, char *argv[])
{

	LLFS_Interface *LLFS = OpenLLFS();
	LLFS_Init(LLFS);

	printf("Init complete\n");
	char data[BLOCK_SIZE];
	memset(data, 0, 512);
	for (int i = 0; i < 255; i++) data[i] = i;
	char* name = "TestFile";
	LLFS_new(LLFS, name, data, 512);

	for (int i = 0; i < 255; i++) data[i] = 255-i;
	printf("Checking buffer for Testfile2.txt: return %d\n", checkInBuffer(LLFS, name, strlen(name)));
	name = "TestFile2.txt";
	LLFS_new(LLFS, name, data, 512);

	name = "TestDirectory";
	LLFS_mkdir(LLFS, name);



	LLFS_ls(LLFS);

	char * path = "/TestDirectory/";
	LLFS_cd(LLFS, path);

	printf("Directory created\n");

	name = "SubDirectory";
	LLFS_mkdir(LLFS, name);

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
#endif
