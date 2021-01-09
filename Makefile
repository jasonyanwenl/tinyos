BUILD_DIR = ./build
HD_IMG = ./hd60M.img
ENTRY_POINT = 0xc0001500
AS = nasm
CC = gcc
LD = ld
LIB = -I lib/ -I lib/kernel/ -I lib/user/ -I kernel/ -I device/ -I thread/ -I userprog/
BOOT_LIB = -I boot/include/
ASFLAGS = -f elf
CFLAGS = -Wall $(LIB) -c -fno-builtin -W -Wstrict-prototypes -Wmissing-prototypes
LDFLAGS = -Ttext $(ENTRY_POINT) -e main -Map $(BUILD_DIR)/kernel.map
OBJS = $(BUILD_DIR)/main.o $(BUILD_DIR)/init.o $(BUILD_DIR)/interrupt.o \
		$(BUILD_DIR)/timer.o $(BUILD_DIR)/kernel.o $(BUILD_DIR)/print.o \
		$(BUILD_DIR)/debug.o $(BUILD_DIR)/memory.o $(BUILD_DIR)/bitmap.o \
		$(BUILD_DIR)/string.o $(BUILD_DIR)/thread.o $(BUILD_DIR)/switch.o \
		$(BUILD_DIR)/list.o $(BUILD_DIR)/console.o $(BUILD_DIR)/sync.o \
		$(BUILD_DIR)/keyboard.o $(BUILD_DIR)/ioqueue.o $(BUILD_DIR)/tss.o \
		$(BUILD_DIR)/process.o $(BUILD_DIR)/syscall.o $(BUILD_DIR)/syscall-init.o \
		$(BUILD_DIR)/stdio.o $(BUILD_DIR)/stdio-kernel.o $(BUILD_DIR)/ide.o

all: mk_dir build hd

################ Comiple C ###############
$(BUILD_DIR)/main.o: kernel/main.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/init.o: kernel/init.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/interrupt.o: kernel/interrupt.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/timer.o: device/timer.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/debug.o: kernel/debug.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/memory.o: kernel/memory.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/bitmap.o: lib/kernel/bitmap.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/string.o: lib/string.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/thread.o: thread/thread.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/list.o: lib/kernel/list.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/console.o: device/console.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/sync.o: thread/sync.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/keyboard.o: device/keyboard.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/ioqueue.o: device/ioqueue.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/tss.o: userprog/tss.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/process.o: userprog/process.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/syscall.o: lib/user/syscall.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/syscall-init.o: userprog/syscall-init.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/stdio.o: lib/stdio.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/stdio-kernel.o: lib/kernel/stdio-kernel.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/ide.o: device/ide.c
	$(CC) $(CFLAGS) $< -o $@

############# Comiple Assembly ###########
$(BUILD_DIR)/mbr.bin: boot/mbr.S
	$(AS) $(BOOT_LIB) $< -o $@

$(BUILD_DIR)/loader.bin: boot/loader.S
	$(AS) $(BOOT_LIB) $< -o $@

$(BUILD_DIR)/kernel.o: kernel/kernel.S
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/print.o: lib/kernel/print.S
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/switch.o: thread/switch.S
	$(AS) $(ASFLAGS) $< -o $@

################# Link ###################
$(BUILD_DIR)/kernel.bin: $(OBJS)
	echo kernel.bin
	$(LD) $(LDFLAGS) $^ -o $@

.PHONY: mk_dir hd clean all

mk_dir:
	if [[ ! -d $(BUILD_DIR) ]]; then mkdir $(BUILD_DIR); fi

hd:
	dd if=$(BUILD_DIR)/mbr.bin of=$(HD_IMG) bs=512 count=1 conv=notrunc
	dd if=$(BUILD_DIR)/loader.bin of=$(HD_IMG) bs=512 count=4 seek=2 conv=notrunc
	dd if=$(BUILD_DIR)/kernel.bin of=$(HD_IMG) bs=512 count=200 seek=9 conv=notrunc

clean:
	cd $(BUILD_DIR) && rm -f ./*

build: $(BUILD_DIR)/mbr.bin $(BUILD_DIR)/loader.bin $(BUILD_DIR)/kernel.bin