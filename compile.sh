#!/bin/bash
cd bin
gcc -o controller ../controller.c -D_FILE_OFFSET_BITS=64 -Wall `pkg-config fuse --cflags --libs`
echo "compile complete"
