# Compiler & linker
ASM           = nasm
LIN           = ld
CC            = gcc

# Directory
SOURCE_FOLDER = src
OUTPUT_FOLDER = bin
ISO_NAME      = OS2023

# Flags
WARNING_CFLAG = -Wall -Wextra -Werror
DEBUG_CFLAG   = -ffreestanding -fshort-wchar -g
STRIP_CFLAG   = -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs
CFLAGS        = $(DEBUG_CFLAG) $(WARNING_CFLAG) $(STRIP_CFLAG) -m32 -c -I$(SOURCE_FOLDER)
AFLAGS        = -f elf32 -g -F dwarf
LFLAGS        = -T $(SOURCE_FOLDER)/linker.ld -melf_i386

run: all
	@qemu-system-i386 -s -drive file=bin/storage.bin,format=raw,if=ide,index=0,media=disk -cdrom $(OUTPUT_FOLDER)/$(ISO_NAME).iso
all: build
build: iso
clean:
	@rm -r $(OUTPUT_FOLDER)/*



kernel:
	@$(ASM) $(AFLAGS) $(SOURCE_FOLDER)/kernel_loader.s -o $(OUTPUT_FOLDER)/kernel_loader.o

	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/stdmem.c -o $(OUTPUT_FOLDER)/stdmem.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/portio.c -o $(OUTPUT_FOLDER)/portio.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/framebuffer.c -o $(OUTPUT_FOLDER)/framebuffer.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/gdt.c -o $(OUTPUT_FOLDER)/gdt.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/kernel.c -o $(OUTPUT_FOLDER)/kernel.o

	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/interrupt/idt.c -o $(OUTPUT_FOLDER)/idt.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/interrupt/interrupt.c -o $(OUTPUT_FOLDER)/interrupt.o
	@$(ASM) $(AFLAGS) $(SOURCE_FOLDER)/interrupt/intsetup.s -o $(OUTPUT_FOLDER)/intsetup.o			

	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/keyboard/keyboard.c -o $(OUTPUT_FOLDER)/keyboard.o

	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/filesystem/fat32.c -o $(OUTPUT_FOLDER)/fat32.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/filesystem/disk.c -o $(OUTPUT_FOLDER)/disk.o

	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/paging/paging.c -o $(OUTPUT_FOLDER)/paging.o

	@$(LIN) $(LFLAGS) bin/*.o -o $(OUTPUT_FOLDER)/kernel

	@echo Linking object files and generate elf32...
	@rm -f *.o

iso: kernel
	@mkdir -p $(OUTPUT_FOLDER)/iso/boot/grub
	@cp $(OUTPUT_FOLDER)/kernel     $(OUTPUT_FOLDER)/iso/boot/
	@cp other/grub1                 $(OUTPUT_FOLDER)/iso/boot/grub/
	@cp $(SOURCE_FOLDER)/menu.lst   $(OUTPUT_FOLDER)/iso/boot/grub/
	genisoimage -R  		\
		-b boot/grub/grub1  \
		-no-emul-boot       \
		-boot-load-size 4   \
		-A os               \
		-input-charset utf8 \
		-quiet              \
		-boot-info-table    \
		-o bin/$(ISO_NAME).iso   \
		$(OUTPUT_FOLDER)/iso
	@rm -r $(OUTPUT_FOLDER)/iso


DISK_NAME      = storage
disk:
	@qemu-img create -f raw $(OUTPUT_FOLDER)/$(DISK_NAME).bin 4M

inserter:
	@$(CC) -Wno-builtin-declaration-mismatch -g \
		$(SOURCE_FOLDER)/stdmem.c $(SOURCE_FOLDER)/filesystem/fat32.c \
		$(SOURCE_FOLDER)/inserter/external-inserter.c \
		-o $(OUTPUT_FOLDER)/inserter
# how to run : ./inserter <file to insert> <parent cluster index> <storage>

user-shell:
	@$(ASM) $(AFLAGS) $(SOURCE_FOLDER)/user-entry.s -o user-entry.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/user-shell.c -o user-shell.o
	@$(LIN) -T $(SOURCE_FOLDER)/user-linker.ld -melf_i386 \
		user-entry.o user-shell.o -o $(OUTPUT_FOLDER)/shell
	@echo Linking object shell object files and generate flat binary...
	@size --target=binary bin/shell
	@rm -f *.o

insert-shell: inserter user-shell
	@echo Inserting shell into root directory...
    @cd $(OUTPUT_FOLDER); ./inserter shell 2 $(DISK_NAME).bin