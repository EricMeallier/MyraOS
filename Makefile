# === Configurable Variables ===
ASM           = nasm
CC            = i386-elf-gcc
LD            = i386-elf-ld
QEMU          = qemu-system-i386

BOOT_SRC      = boot/boot.asm
BOOT_BIN      = boot/boot.bin

KERNEL_SRC    = kernel/kernel.c
KERNEL_OBJ    = kernel/kernel.o
KERNEL_BIN    = kernel/kernel.bin
LINKER_SCRIPT = linker.ld

FLOPPY_IMG    = build/floppy.img

# === Platform Detection ===
ifeq ($(OS),Windows_NT)
    RM = powershell -Command "Remove-Item -ErrorAction Ignore -Force"
    MKDIR = powershell -Command "New-Item -ItemType Directory -Force -Path build | Out-Null"
    FILE_SIZE = powershell -Command "if ((Get-Item '$(BOOT_BIN)').Length -ne 512) { Write-Error 'Bootloader must be 512 bytes'; exit 1 }"
    CREATE_IMG = powershell -Command "$$f=New-Object byte[] 1474560;[IO.File]::WriteAllBytes('$(FLOPPY_IMG)',$$f)"
    DD_BOOT = powershell -Command "$$b=[IO.File]::ReadAllBytes('$(BOOT_BIN)');$$i=[IO.File]::ReadAllBytes('$(FLOPPY_IMG)');[Array]::Copy($$b,0,$$i,0,512);[IO.File]::WriteAllBytes('$(FLOPPY_IMG)',$$i)"
    DD_KERNEL = powershell -Command "$$k=[IO.File]::ReadAllBytes('$(KERNEL_BIN)');$$i=[IO.File]::ReadAllBytes('$(FLOPPY_IMG)');[Array]::Copy($$k,0,$$i,512,$$k.Length);[IO.File]::WriteAllBytes('$(FLOPPY_IMG)',$$i)"
else
    RM = rm -f
    MKDIR = mkdir -p build
    FILE_SIZE = @if [ $$(stat -f%z $(BOOT_BIN)) -ne 512 ]; then echo "Error: Bootloader must be 512 bytes"; exit 1; fi
    CREATE_IMG = dd if=/dev/zero of=$(FLOPPY_IMG) bs=512 count=2880 status=none
    DD_BOOT = dd if=$(BOOT_BIN) of=$(FLOPPY_IMG) bs=512 count=1 conv=notrunc status=none
    DD_KERNEL = dd if=$(KERNEL_BIN) of=$(FLOPPY_IMG) bs=512 seek=1 conv=notrunc status=none
endif

# === Targets ===
all: run

$(BOOT_BIN): $(BOOT_SRC)
	@echo [NASM] Assembling bootloader...
	@$(ASM) -f bin $(BOOT_SRC) -o $(BOOT_BIN)
	@$(MKDIR)
	@$(FILE_SIZE)

$(KERNEL_OBJ): $(KERNEL_SRC)
	@echo [GCC] Compiling kernel...
	@$(CC) -ffreestanding -m32 -c $(KERNEL_SRC) -o $(KERNEL_OBJ)

$(KERNEL_BIN): $(KERNEL_OBJ) $(LINKER_SCRIPT)
	@echo [LD] Linking kernel...
	@$(LD) -T $(LINKER_SCRIPT) -o $(KERNEL_BIN) -nostdlib $(KERNEL_OBJ) -m elf_i386

$(FLOPPY_IMG): $(BOOT_BIN) $(KERNEL_BIN)
	@echo [IMG] Creating floppy image...
	@$(MKDIR)
	@$(CREATE_IMG)
	@$(DD_BOOT)
	@$(DD_KERNEL)

run: $(FLOPPY_IMG)
	@echo [QEMU] Launching OS...
	@$(QEMU) -drive format=raw,file=$(FLOPPY_IMG),if=floppy

clean:
	@echo [CLEAN] Removing build artifacts...
	@-$(RM) $(BOOT_BIN) $(KERNEL_OBJ) $(KERNEL_BIN) $(FLOPPY_IMG)

.PHONY: all run clean