# Youcto App

A GPIO testing application for aarch64 platform.

## Dependencies
- GTK+ 3.0
- gcc-aarch64-linux-gnu
- pkg-config

## Build Instructions

1. Install cross-compilation tools on Ubuntu/Debian host:
```bash
sudo apt-get update
sudo apt-get install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu pkg-config libgtk-3-dev
```

2. Using the provided compile script:
```bash
chmod +x compile.sh
./compile.sh
```

Or compile manually:
```bash
make clean
make
```

## Deployment

Copy the compiled `launcher` executable to your target aarch64 device:
```bash
scp launcher user@target-device:/path/to/destination/
```

## Running the Application

Execute on the target device:
```bash
./launcher
```

Note: Make sure GTK+ 3.0 runtime is installed on the target device:
```bash
apt-get install libgtk-3-0
