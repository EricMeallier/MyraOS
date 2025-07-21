# ──────────  Toolchain  ──────────
ASM  := nasm
CC   := i386-elf-gcc
LD   := i386-elf-ld
QEMU := qemu-system-i386

# ──────────  Paths  ──────────
SRC_DIR        := system
OBJ_DIR        := build/obj
ISO_DIR        := build/iso
FS_IMG 		   := build/fs.img
ISO_IMG        := build/MyraOS.iso
KERNEL_ELF     := build/kernel.elf
LINKER_SCRIPT  := linker.ld

# ──────────  Source discovery  ──────────
SRC_FILES  := $(shell find $(SRC_DIR) -name '*.c')
OBJ_FILES  := $(patsubst $(SRC_DIR)/%, $(OBJ_DIR)/%, $(SRC_FILES:.c=.o))

INCLUDE_DIRS  := $(shell find $(SRC_DIR) -type d -name include)
INCLUDE_FLAGS := $(foreach d,$(INCLUDE_DIRS),-I"$d")

# All .asm sources
ASM_FILES      := $(shell find $(SRC_DIR) -name '*.asm')
ASM_OBJ_FILES  := $(patsubst $(SRC_DIR)/%, $(OBJ_DIR)/%, \
                   $(ASM_FILES:.asm=.o))

ALL_OBJS := $(OBJ_FILES) $(ASM_OBJ_FILES)

# ──────────  Configurable Flags  ──────────
DEBUG ?= 0

CFLAGS := -ffreestanding -m32 -std=gnu99 -Wall -Wextra -fno-omit-frame-pointer $(INCLUDE_FLAGS)
ifeq ($(DEBUG), 1)
CFLAGS += -g -O0
else
CFLAGS += -O2
endif

# ──────────  Default target  ──────────
all: run

# ──────────  Build rules  ──────────
# C
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "[GCC] $<"
	@mkdir -p $(dir $@)
	@$(CC) -g -ffreestanding -m32 -std=gnu99 -O2 -Wall -Wextra \
			-Wall -Wextra -fno-omit-frame-pointer \
			$(INCLUDE_FLAGS) -c $< -o $@

# Assembly
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.asm
	@echo "[NASM] $<"
	@mkdir -p $(dir $@)
	@$(ASM) -f elf32 $< -o $@

$(FS_IMG):
	@echo "[FS] Creating ext2 disk image"
	@dd if=/dev/zero of=$(FS_IMG) bs=1M count=32
	@mkfs.ext2 $(FS_IMG)

# Kernel ELF
$(KERNEL_ELF): $(ALL_OBJS) $(LINKER_SCRIPT)
	@echo "[LD] Linking kernel ELF"
	@mkdir -p $(dir $@)
	@$(LD) -T $(LINKER_SCRIPT) -nostdlib -m elf_i386 \
	       -o $@ $(ALL_OBJS)

# ──────────  GRUB ISO  ──────────
GRUB_CFG := $(SRC_DIR)/boot/grub/grub.cfg

$(ISO_IMG): $(KERNEL_ELF) $(GRUB_CFG)
	@echo "[ISO] Creating GRUB ISO"
	@mkdir -p $(ISO_DIR)/boot/grub
	@cp $(KERNEL_ELF) $(ISO_DIR)/boot/kernel.elf
	@cp $(GRUB_CFG)   $(ISO_DIR)/boot/grub/grub.cfg
	@grub2-mkrescue -o $(ISO_IMG) $(ISO_DIR) >/dev/null 2>&1

# ──────────  Run targets  ──────────
run: $(ISO_IMG) $(FS_IMG)
	@echo "[QEMU] Running MyraOS"
	@$(QEMU) -cdrom $(ISO_IMG) \
	        -drive file=$(FS_IMG),format=raw,if=ide,index=0 \
	        -m 128M

debug: DEBUG=1
debug: $(ISO_IMG) $(FS_IMG)
	@echo "[QEMU] Running MyraOS (GDB mode)"
	@$(QEMU) -s -S -cdrom $(ISO_IMG) \
	        -drive file=$(FS_IMG),format=raw,if=ide,index=0 \
	        -m 128M

# ──────────  Clean  ──────────
clean:
	@echo "[CLEAN] Removing build artifacts"
	@rm -rf build
