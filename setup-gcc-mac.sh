#!/bin/bash

set -e

# Step 1: Install required packages with Homebrew
echo "Installing dependencies..."
brew install nasm qemu curl gmp mpfr libmpc texinfo

# Step 2: Set environment variables
export PREFIX="/usr/local/i386elfgcc"
export TARGET=i386-elf
export PATH="$PREFIX/bin:$PATH"

# Step 3: Prepare source directory
mkdir -p /tmp/src
cd /tmp/src

# Step 4: Build binutils
echo "Downloading binutils..."
curl -LO http://ftp.gnu.org/gnu/binutils/binutils-2.39.tar.gz
tar xf binutils-2.39.tar.gz
mkdir binutils-build
cd binutils-build

echo "Configuring binutils..."
../binutils-2.39/configure --target=$TARGET --prefix=$PREFIX --disable-nls

echo "Building binutils..."
make -j$(sysctl -n hw.ncpu)
sudo make install

# Step 5: Build GCC
cd /tmp/src
echo "Downloading GCC..."
curl -LO https://ftp.gnu.org/gnu/gcc/gcc-12.2.0/gcc-12.2.0.tar.gz
tar xf gcc-12.2.0.tar.gz
mkdir gcc-build
cd gcc-build

echo "Configuring GCC..."
../gcc-12.2.0/configure --target=$TARGET --prefix=$PREFIX --disable-nls --disable-libssp --enable-languages=c,c++ --without-headers

echo "Building GCC (may take a while)..."
make all-gcc -j$(sysctl -n hw.ncpu)
make all-target-libgcc -j$(sysctl -n hw.ncpu)

echo "Installing GCC..."
sudo make install-gcc
sudo make install-target-libgcc

# Step 6: Verify
echo "Done! Installed tools:"
ls $PREFIX/bin
echo "GCC Version:"
$PREFIX/bin/i386-elf-gcc --version