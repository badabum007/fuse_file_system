#!/bin/bash
rm controller
gcc -c ../fs.c -w -std=gnu99
gcc -c ../controller.c  -D_FILE_OFFSET_BITS=64 -Wall `pkg-config fuse --cflags --libs` -std=gnu99 -w
gcc -o controller controller.o fs.o -D_FILE_OFFSET_BITS=64 -Wall `pkg-config fuse --cflags --libs` -std=gnu99 -w
