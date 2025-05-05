# === Variables ===
ASM=nasm
QEMU=qemu-system-i386
BOOT_SRC=boot/boot.asm
BOOT_BIN=boot/boot.bin
FLOPPY_IMG=build/floppy.img

# === Rules ===

all: run

# Assemble bootloader
$(BOOT_BIN): $(BOOT_SRC)
	@echo "[NASM] Assembling bootloader..."
	$(ASM) -f bin $(BOOT_SRC) -o $(BOOT_BIN)
	@size=$$(stat -c%s "$(BOOT_BIN)"); \
	if [ $$size -ne 512 ]; then \
		echo "Error: Bootloader must be exactly 512 bytes (got $$size)"; \
		exit 1; \
	fi

# Create floppy image and write bootloader to it
$(FLOPPY_IMG): $(BOOT_BIN)
	@echo "[IMG] Creating floppy image..."
	@mkdir -p $(dir $(FLOPPY_IMG))
	@dd if=/dev/zero of=$(FLOPPY_IMG) bs=512 count=2880 status=none
	@dd if=$(BOOT_BIN) of=$(FLOPPY_IMG) bs=512 count=1 conv=notrunc status=none

# Run in QEMU
run: $(FLOPPY_IMG)
	@echo "[QEMU] Launching..."
	$(QEMU) -drive format=raw,file=$(FLOPPY_IMG),if=floppy

# Clean build artifacts
clean:
	@echo "[CLEAN] Removing build artifacts..."
	rm -f $(BOOT_BIN) $(FLOPPY_IMG)

.PHONY: all run clean
