#!/bin/bash
cd bin
rm controller
gcc -o controller ../controller.c -D_FILE_OFFSET_BITS=64 -Wall `pkg-config fuse --cflags --libs` -std=gnu99 -w
echo "compile complete"
