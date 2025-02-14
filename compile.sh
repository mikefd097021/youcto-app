#!/bin/bash

# Install required cross-compilation tools and dependencies
sudo apt-get update
sudo apt-get install -y \
    gcc-aarch64-linux-gnu \
    g++-aarch64-linux-gnu \
    pkg-config \
    libgtk-3-dev

# Set PKG_CONFIG_PATH
export PKG_CONFIG_PATH=/usr/lib/aarch64-linux-gnu/pkgconfig

# Run compilation
make clean
make

echo "Compilation complete!"