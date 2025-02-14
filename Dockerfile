FROM ubuntu:22.04

# Install required packages
RUN apt-get update && apt-get install -y \
    gcc-aarch64-linux-gnu \
    g++-aarch64-linux-gnu \
    pkg-config \
    libgtk-3-dev \
    make \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Set PKG_CONFIG_PATH for cross-compilation
ENV PKG_CONFIG_PATH=/usr/lib/aarch64-linux-gnu/pkgconfig

# Copy source files
COPY . .

# Default command
CMD ["make"]