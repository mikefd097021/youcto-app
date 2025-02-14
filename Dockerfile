FROM ubuntu:22.04

# Install required packages
RUN dpkg --add-architecture arm64 && \
    apt-get update && \
    apt-get install -y \
    gcc-aarch64-linux-gnu \
    g++-aarch64-linux-gnu \
    pkg-config \
    libgtk-3-dev:arm64 \
    libc6-dev-arm64-cross \
    linux-libc-dev-arm64-cross \
    make \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Set cross-compilation environment variables
ENV PKG_CONFIG_PATH=/usr/lib/aarch64-linux-gnu/pkgconfig
ENV PKG_CONFIG_SYSROOT_DIR=/usr/aarch64-linux-gnu
ENV PKG_CONFIG_LIBDIR=/usr/lib/aarch64-linux-gnu/pkgconfig

# Copy source files
COPY . .

# Default command
CMD ["bash", "compile.sh"]