CC=gcc
CFLAGS := -g -Wall -Wno-deprecated-declarations -Werror

all: File

clean:
	rm -rf File File.dSYM

File: disk/DiskController.c disk/DiskController.h io/File.c io/File.h 
	$(CC) $(CFLAGS) -o File io/File.c disk/DiskController.c