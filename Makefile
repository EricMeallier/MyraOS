# === Toolchain ===
ASM = nasm
CC = i386-elf-gcc
LD = i386-elf-ld
QEMU = qemu-system-i386

# === Paths ===
SRC_DIR = src
OBJ_DIR = build/obj
BOOT_SRC = $(SRC_DIR)/boot/boot.asm
BOOT_BIN = build/boot.bin
KERNEL_BIN = build/kernel.bin
LINKER_SCRIPT = linker.ld
FLOPPY_IMG = build/floppy.img

# === Source discovery ===
ifeq ($(OS),Windows_NT)
    SRC_FILES := $(shell powershell -Command "Get-ChildItem -Path $(SRC_DIR) -Recurse -Filter *.c | ForEach-Object { $$_.FullName }")
else
    SRC_FILES := $(shell find $(SRC_DIR) -name '*.c')
endif

SRC_FILES := $(subst \,/,$(SRC_FILES))
OBJ_FILES := $(patsubst $(SRC_DIR)/%, $(OBJ_DIR)/%, $(SRC_FILES:.c=.o))

# === OS-dependent helpers ===
ifeq ($(OS),Windows_NT)
    RM = powershell -Command "Remove-Item -Recurse -ErrorAction Ignore -Force"
    MKDIR = powershell -Command "New-Item -ItemType Directory -Force -Path"
    FILE_SIZE = powershell -Command "if ((Get-Item '$(BOOT_BIN)').Length -ne 512) { Write-Error 'Bootloader must be 512 bytes'; exit 1 }"
    CREATE_IMG = powershell -Command "$$f=New-Object byte[] 1474560;[IO.File]::WriteAllBytes('$(FLOPPY_IMG)',$$f)"
    DD_BOOT = powershell -Command "$$b=[IO.File]::ReadAllBytes('$(BOOT_BIN)');$$i=[IO.File]::ReadAllBytes('$(FLOPPY_IMG)');[Array]::Copy($$b,0,$$i,0,512);[IO.File]::WriteAllBytes('$(FLOPPY_IMG)',$$i)"
    DD_KERNEL = powershell -Command "$$k=[IO.File]::ReadAllBytes('$(KERNEL_BIN)');$$i=[IO.File]::ReadAllBytes('$(FLOPPY_IMG)');[Array]::Copy($$k,0,$$i,512,$$k.Length);[IO.File]::WriteAllBytes('$(FLOPPY_IMG)',$$i)"
else
    RM = rm -rf
    MKDIR = mkdir -p
    FILE_SIZE = @if [ $$(wc -c < "$(BOOT_BIN)") -ne 512 ]; then echo "Error: Bootloader must be 512 bytes"; exit 1; fi
    CREATE_IMG = dd if=/dev/zero of="$(FLOPPY_IMG)" bs=512 count=2880 status=none
    DD_BOOT = dd if="$(BOOT_BIN)" of="$(FLOPPY_IMG)" bs=512 count=1 conv=notrunc status=none
    DD_KERNEL = dd if="$(KERNEL_BIN)" of="$(FLOPPY_IMG)" bs=512 seek=1 conv=notrunc status=none
endif

# === Targets ===
all: run

# Bootloader
$(BOOT_BIN): $(BOOT_SRC)
	@echo [NASM] Assembling bootloader...
	@$(MKDIR) build
	@$(ASM) -f bin $< -o $@
	@$(FILE_SIZE)

# Compile each .c to .o with folder structure preserved
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo [GCC] Compiling $<
	@$(MKDIR) $(dir $@)
	@$(CC) -ffreestanding -m32 -I$(SRC_DIR) -c $< -o $@ -Wall -Wextra -std=gnu99 -O0 -ggdb

# Link kernel
$(KERNEL_BIN): $(OBJ_FILES) $(LINKER_SCRIPT)
	@echo [LD] Linking kernel...
	@$(LD) -T $(LINKER_SCRIPT) -o $@ $(OBJ_FILES) -nostdlib -m elf_i386

# Floppy image
$(FLOPPY_IMG): $(BOOT_BIN) $(KERNEL_BIN)
	@echo [IMG] Creating floppy image...
	@$(MKDIR) build
	@$(CREATE_IMG)
	@$(DD_BOOT)
	@$(DD_KERNEL)

# Run
run: $(FLOPPY_IMG)
	@echo [QEMU] Launching OS...
	@$(QEMU) -drive format=raw,file="$(FLOPPY_IMG)",if=floppy

clean:
	@echo [CLEAN] Removing build artifacts...
	@-$(RM) build

.PHONY: all run clean