#!/bin/bash
rm format
gcc -c fs.c -w -std=gnu99
gcc -c format.c -std=gnu99 -w
gcc -o format format.o fs.o -std=gnu99 -w
rm *.o