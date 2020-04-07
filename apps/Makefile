CC:=gcc
CFLAGS:=-g -Wall -Werror 

TESTFILES := $(wildcard test*.c) 
$(info TESTFILES are $(TESTFILES))

TESTS := $(TESTFILES:%.c=%)
$(info TESTS are $(TESTS))

all: $(TESTS) disk.o file.o

test%: test%.c file.o disk.o
	$(CC) $(CFLAGS) -o ./$@ $^

disk.o: ./../disk/DiskController.c ./../disk/DiskController.h
	$(CC) $(CFLAGS) -c -o $@ $<

file.o: ./../io/File.c ./../io/File.h
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean

