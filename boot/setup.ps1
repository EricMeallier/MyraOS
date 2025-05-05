# Get the root project directory (one level above /boot)
$Root = Resolve-Path "$PSScriptRoot\.."

# Define paths relative to the root
$asmFile = "$Root\boot\boot.asm"
$binFile = "$Root\boot\boot.bin"
$imgFile = "$Root\build\floppy.img"

# 1. Assemble boot.asm to boot.bin
Write-Host "Assembling $asmFile..."
nasm -f bin $asmFile -o $binFile

if (-not (Test-Path $binFile)) {
    Write-Error "Assembly failed: $binFile not generated."
    exit 1
}

# 2. Check size
if ((Get-Item $binFile).Length -ne 512) {
    Write-Error "$binFile is not 512 bytes! Build failed."
    exit 1
}

# 3. Create blank 1.44MB floppy image
Write-Host "Creating blank floppy image..."
Remove-Item -Force $imgFile -ErrorAction Ignore
fsutil file createnew $imgFile 1474560 | Out-Null

# 4. Inject boot.bin into first 512 bytes
Write-Host "Injecting bootloader..."
$boot = [System.IO.File]::ReadAllBytes($binFile)
$img = [System.IO.File]::ReadAllBytes($imgFile)

if (-not $boot) { Write-Error "Failed to load $binFile"; exit 1 }
if (-not $img) { Write-Error "Failed to load $imgFile"; exit 1 }

[Array]::Copy($boot, 0, $img, 0, 512)
[System.IO.File]::WriteAllBytes($imgFile, $img)

# 5. Run with QEMU
Write-Host "Launching QEMU..."
qemu-system-i386 -drive format=raw,file=$imgFile,if=floppy
