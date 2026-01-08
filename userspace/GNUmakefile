# userspace/Makefile – NTux-OS Userspace (FINAL – direkt als Limine-Modul)
CC      := gcc
LD      := $(CC)
CFLAGS  := -std=gnu11 \
            -I. -I ../-I../libc \
            -ffreestanding -fno-pic -fno-stack-protector \
            -mno-red-zone -mcmodel=large \
            -O2 -Wall -Wextra

LDFLAGS := -nostdlib -static -no-pie -T linker.ld -z max-page-size=0x2000

# Alle Programme
PROGRAMS := init.elf

# Quellen
C_SRCS   := $(wildcard *.c)
OBJS     := $(C_SRCS:.c=.o)

.PHONY: all clean

all: $(PROGRAMS)
	@echo "Userspace gebaut: init.elf"
	@mkdir -p ../iso_root/boot
	@cp init.elf ../iso_root/boot/init.elf
	# DIREKT INS ISO!
	@echo "init.elf → iso_root/boot/init.elf"

init.elf: $(OBJS)
	@echo "Linking init.elf ..."
	$(LD) $(LDFLAGS) $^ -o $@

%.o: %.c
	@echo "Compiling $< ..."
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *.elf