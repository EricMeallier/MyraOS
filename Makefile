# === Toolchain ===
ASM  = nasm
CC   = i386-elf-gcc
LD   = i386-elf-ld
QEMU = qemu-system-i386

# === Paths ===
SRC_DIR        = system
OBJ_DIR        = build/obj
BOOT_SRC       = $(SRC_DIR)/boot/boot.asm
BOOT_BIN       = build/boot.bin
KERNEL_BIN     = build/kernel.bin
KERNEL_ELF     = build/kernel.elf
LINKER_SCRIPT  = linker.ld
FLOPPY_IMG     = build/floppy.img

# === Source discovery ===
SRC_FILES := $(shell find $(SRC_DIR) -name '*.c')
OBJ_FILES := $(patsubst $(SRC_DIR)/%, $(OBJ_DIR)/%, $(SRC_FILES:.c=.o))

INCLUDE_DIRS := $(shell find $(SRC_DIR) -type d -name include)
INCLUDE_FLAGS := $(foreach dir, $(INCLUDE_DIRS), -I"$(dir)")

ASM_FILES := $(filter-out $(BOOT_SRC), $(shell find $(SRC_DIR) -name '*.asm'))
ASM_OBJ_FILES := $(patsubst $(SRC_DIR)/%, $(OBJ_DIR)/%, $(ASM_FILES:.asm=.o))

# === OS-independent helpers (Unix only) ===
RM          = rm -rf
MKDIR       = mkdir -p
FILE_SIZE   = @if [ $$(wc -c < "$(BOOT_BIN)") -ne 512 ]; then echo "Error: Bootloader must be 512 bytes"; exit 1; fi
CREATE_IMG  = dd if=/dev/zero of="$(FLOPPY_IMG)" bs=512 count=2880 status=none
DD_BOOT     = dd if="$(BOOT_BIN)" of="$(FLOPPY_IMG)" bs=512 count=1 conv=notrunc status=none
DD_KERNEL   = dd if="$(KERNEL_BIN)" of="$(FLOPPY_IMG)" bs=512 seek=1 conv=notrunc status=none

# === Targets ===
all: run

# Bootloader
$(BOOT_BIN): $(BOOT_SRC)
	@echo [NASM] Assembling bootloader...
	@$(MKDIR) build
	@$(ASM) -f bin $< -o $@
	@$(FILE_SIZE)

# Compile C sources
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo [GCC] Compiling $<
	@$(MKDIR) $(dir $@)
	@$(CC) -ffreestanding -m32 $(INCLUDE_FLAGS) \
		-c $< -o $@ -Wall -Wextra -std=gnu99 -O0 -ggdb

# Compile asm sources
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.asm
	@echo [NASM] Assembling $<
	@$(MKDIR) $(dir $@)
	@$(ASM) -f elf $< -o $@

# Link raw kernel binary (used for floppy)
$(KERNEL_BIN): $(OBJ_FILES) $(ASM_OBJ_FILES) $(LINKER_SCRIPT)
	@echo [LD] Linking kernel BIN...
	@$(LD) -T $(LINKER_SCRIPT) -o $@ $(patsubst %,'%',$(OBJ_FILES) $(ASM_OBJ_FILES)) -nostdlib -m elf_i386

# Link ELF kernel (for debugging with symbols)
$(KERNEL_ELF): $(OBJ_FILES) $(ASM_OBJ_FILES) $(LINKER_SCRIPT)
	@echo [LD] Linking kernel ELF (with symbols)...
	@$(LD) -T $(LINKER_SCRIPT) -o $@ $(patsubst %,'%',$(OBJ_FILES) $(ASM_OBJ_FILES)) -nostdlib -m elf_i386
	
# Floppy image
$(FLOPPY_IMG): $(BOOT_BIN) $(KERNEL_BIN)
	@echo [IMG] Creating floppy image...
	@$(MKDIR) build
	@$(CREATE_IMG)
	@$(DD_BOOT)
	@$(DD_KERNEL)

# Run in QEMU normally
run: $(FLOPPY_IMG)
	@echo [QEMU] Launching OS...
	@$(QEMU) -drive format=raw,file="$(FLOPPY_IMG)",if=floppy

# Run in QEMU with GDB support
debug: $(FLOPPY_IMG) $(KERNEL_ELF)
	@echo [QEMU] Launching QEMU in GDB mode...
	@$(QEMU) -s -S -drive format=raw,file="$(FLOPPY_IMG)",if=floppy

# Clean build
clean:
	@echo [CLEAN] Removing build artifacts...
	@$(RM) build
