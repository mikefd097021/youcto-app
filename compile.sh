#!/bin/bash

# Install required cross-compilation tools and dependencies
sudo apt-get update
sudo apt-get install -y \
    gcc-aarch64-linux-gnu \
    g++-aarch64-linux-gnu \
    pkg-config \
    libgtk-3-dev:arm64 \
    libc6-dev-arm64-cross \
    linux-libc-dev-arm64-cross

# Set cross-compilation environment variables
export PKG_CONFIG_PATH=/usr/lib/aarch64-linux-gnu/pkgconfig
export PKG_CONFIG_SYSROOT_DIR=/usr/aarch64-linux-gnu
export PKG_CONFIG_LIBDIR=/usr/lib/aarch64-linux-gnu/pkgconfig

# Run compilation
make clean
make

echo "Compilation complete!"