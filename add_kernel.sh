#!/bin/bash

set -e

# clear
rm -rf output/*

# compile mbr
nasm -I boot/include/ -o output/mbr.bin boot/mbr.S

# compile loader
nasm -I boot/include/ -o output/loader.bin boot/loader.S

# compile kernel
gcc -m32 -I lib/kernel/ -c -o output/main.o kernel/main.c

# compile kernel lib
nasm -f elf -o output/print.o lib/kernel/print.S

# link kernel & lib
ld -m elf_i386 -Ttext 0xc0001500 -e main -o output/kernel.bin output/main.o output/print.o

# write to disk
dd if=/home/tinyos/src/output/mbr.bin of=/home/tinyos/src/hd60M_foo.img bs=512 count=1 conv=notrunc
dd if=/home/tinyos/src/output/loader.bin of=/home/tinyos/src/hd60M.img bs=512 count=4 seek=2 conv=notrunc
dd if=/home/tinyos/src/output/kernel.bin of=/home/tinyos/src/hd60M.img bs=512 count=200 seek=9 conv=notrunc
