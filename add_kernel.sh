#!/bin/bash

set -e

# clear
rm -rf build/*

# compile mbr
nasm -I boot/include/ -o build/mbr.bin boot/mbr.S

# compile loader
nasm -I boot/include/ -o build/loader.bin boot/loader.S

# compile kernel
gcc -m32 -I lib/kernel/ -I lib/ -I kernel/ -c -fno-builtin -o build/main.o kernel/main.c
gcc -m32 -I lib/kernel/ -I lib/ -I kernel/ -c -fno-builtin -o build/interrupt.o kernel/interrupt.c
gcc -m32 -I lib/kernel/ -I lib/ -I kernel/ -c -fno-builtin -o build/init.o kernel/init.c
nasm -f elf -o build/kernel.o kernel/kernel.S

# compile kernel lib
nasm -f elf -o build/print.o lib/kernel/print.S

# link kernel & lib
ld -m elf_i386 -Ttext 0xc0001500 -e main -o \
    build/kernel.bin build/main.o build/print.o build/interrupt.o build/init.o \
    build/kernel.o

# write to disk
dd if=/home/tinyos/src/build/mbr.bin of=/home/tinyos/src/hd60M_foo.img bs=512 count=1 conv=notrunc
dd if=/home/tinyos/src/build/loader.bin of=/home/tinyos/src/hd60M.img bs=512 count=4 seek=2 conv=notrunc
dd if=/home/tinyos/src/build/kernel.bin of=/home/tinyos/src/hd60M.img bs=512 count=200 seek=9 conv=notrunc
