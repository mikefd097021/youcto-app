CC = aarch64-linux-gnu-gcc
CFLAGS = -Wall $(shell pkg-config --cflags gtk+-3.0)
LDFLAGS = $(shell pkg-config --libs gtk+-3.0)
TARGET = launcher

$(TARGET): launcher.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(TARGET)