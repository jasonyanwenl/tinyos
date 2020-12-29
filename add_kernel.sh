#!/bin/bash

set -e

cd /home/tinyos/src/kernel && \
gcc -c -o main.o main.c && \
ld main.o -Ttext 0xc0001500 -e main -o kernel.bin && \
rm main.o && \
mv kernel.bin /home/tinyos/bochs/

cd /home/tinyos/bochs && \
dd if=/home/tinyos/bochs/kernel.bin of=/home/tinyos/bochs/hd60M.img bs=512 count=200 seek=9 conv=notrunc
