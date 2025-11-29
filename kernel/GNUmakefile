# Nuke built-in rules.
.SUFFIXES:

# Final executable name
override OUTPUT := kernel

# Default architecture
ARCH ?= x86_64

# Install prefix
PREFIX := /usr/local

# Check supported architectures
ifeq ($(filter $(ARCH),aarch64 loongarch64 riscv64 x86_64),)
    $(error Architecture $(ARCH) not supported)
endif

# Toolchain selection
TOOLCHAIN :=
TOOLCHAIN_PREFIX :=
ifneq ($(TOOLCHAIN),)
    ifeq ($(TOOLCHAIN_PREFIX),)
        TOOLCHAIN_PREFIX := $(TOOLCHAIN)-
    endif
endif

ifneq ($(TOOLCHAIN_PREFIX),)
    CC  := $(TOOLCHAIN_PREFIX)gcc
    LD  := $(TOOLCHAIN_PREFIX)ld
    ASM := $(TOOLCHAIN_PREFIX)as
else
    CC  := cc
    LD  := ld
    ASM := as
endif

NASM := nasm

# LLVM overrides
ifeq ($(TOOLCHAIN),llvm)
    CC  := clang
    LD  := ld.lld
    ASM := clang -c
endif

# ------------------------------------------------------------------
# User-controllable flags
# ------------------------------------------------------------------
CFLAGS    := -g -O2 -pipe -mno-sse
CPPFLAGS  :=
LDFLAGS   :=
ifeq ($(ARCH),x86_64)
NASMFLAGS := -g
endif

# ------------------------------------------------------------------
# Dependency check
# ------------------------------------------------------------------
ifneq ($(filter-out clean distclean,$(MAKECMDGOALS)),)
    ifeq ($(wildcard .deps-obtained),)
        $(error Please run the ./get-deps script first)
    endif
endif

# ------------------------------------------------------------------
# Internal compiler flags (do NOT change unless you know exactly why)
# ------------------------------------------------------------------
override CFLAGS += \
    -Wall -Wextra \
    -std=gnu11 \
    -nostdinc \
    -ffreestanding \
    -fno-stack-protector \
    -fno-stack-check \
    -fno-lto \
    -fno-PIC \
    -fno-PIE \
    -ffunction-sections \
    -fdata-sections \
    -mno-sse

override CPPFLAGS := \
    -I src \
    -I ../\
    -I limine-protocol/include \
    -isystem freestnd-c-hdrs/include \
    -iquote ../libc \
    $(CPPFLAGS) \
    -MMD -MP

# Detect clang for target triple
CC_IS_CLANG := $(shell $(CC) --version 2>/dev/null | grep -q clang && echo 1 || echo 0)
ifeq ($(CC_IS_CLANG),1)
    override CC += -target $(ARCH)-unknown-none-elf
endif

# Architecture-specific flags
ifeq ($(ARCH),x86_64)
    override CFLAGS += -m64 -march=x86-64 -mabi=sysv -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone -mcmodel=kernel
    override LDFLAGS += -m elf_x86_64
    override NASMFLAGS += -f elf64 -Wall
    ifeq ($(findstring -g,$(NASMFLAGS)),)
        override NASMFLAGS += -g -F dwarf
    endif
endif

# (die anderen Archs kannst du bei Bedarf wieder einfügen)

override LDFLAGS += \
    -nostdlib \
    -static \
    -z max-page-size=0x1000 \
    --gc-sections \
    -T linker-scripts/$(ARCH).lds

# ------------------------------------------------------------------
# Sources & Objects
# ------------------------------------------------------------------
KERNEL_SRC := $(shell find -L src cc-runtime/src -type f ! -path 'src/arch/*' 2>/dev/null | LC_ALL=C sort)
KERNEL_SRC += $(shell find -L src/arch/$(ARCH) -type f 2>/dev/null | LC_ALL=C sort)

LIBC_SRC   := $(shell find ../libc -name '*.c' 2>/dev/null | LC_ALL=C sort)

ALL_C_SRC  := $(filter %.c,$(KERNEL_SRC)) $(LIBC_SRC)
ALL_S_SRC  := $(filter %.S,$(KERNEL_SRC))
ifeq ($(ARCH),x86_64)
ALL_ASM_SRC:= $(filter %.asm,$(KERNEL_SRC))
endif

KERNEL_OBJ := $(patsubst %.c,obj-$(ARCH)/%.c.o,$(filter %.c,$(KERNEL_SRC)))
LIBC_OBJ   := $(patsubst %.c,obj-$(ARCH)/%.k.o,$(LIBC_SRC))
ASM_OBJ    := $(patsubst %.S,obj-$(ARCH)/%.S.o,$(ALL_S_SRC))
NASM_OBJ   := $(patsubst %.asm,obj-$(ARCH)/%.asm.o,$(ALL_ASM_SRC))

ALL_OBJ    := $(KERNEL_OBJ) $(LIBC_OBJ) $(ASM_OBJ)
ifeq ($(ARCH),x86_64)
ALL_OBJ    += $(NASM_OBJ)
endif

ALL_DEPS   := $(ALL_OBJ:.o=.d)

# ------------------------------------------------------------------
# Rules
# ------------------------------------------------------------------
.PHONY: all clean distclean install uninstall

all: bin-$(ARCH)/$(OUTPUT)

bin-$(ARCH)/$(OUTPUT): $(ALL_OBJ) linker-scripts/$(ARCH).lds
	@mkdir -p $(dir $@)
	$(info [LD]  $@)
	$(LD) $(LDFLAGS) $(ALL_OBJ) -o $@

obj-$(ARCH)/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(info [CC]  $<)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

obj-$(ARCH)/%.k.o: %.c
	@mkdir -p $(dir $@)
	$(info [CC]  $< (libc))
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

obj-$(ARCH)/%.S.o: %.S
	@mkdir -p $(dir $@)
	$(info [ASM] $<)
	$(ASM) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

ifeq ($(ARCH),x86_64)
obj-$(ARCH)/%.asm.o: %.asm
	@mkdir -p $(dir $@)
	$(info [NASM] $<)
	$(NASM) $(NASMFLAGS) $< -o $@
endif

-include $(ALL_DEPS)

# ------------------------------------------------------------------
# Clean
# ------------------------------------------------------------------
clean:
	rm -rf bin-$(ARCH) obj-$(ARCH)

distclean: clean
	rm -rf bin-* obj-* .deps-obtained freestnd-c-hdrs cc-runtime limine-protocol

# ------------------------------------------------------------------
# Install
# ------------------------------------------------------------------
install: all
	install -d "$(DESTDIR)$(PREFIX)/share/$(OUTPUT)"
	install -m 644 bin-$(ARCH)/$(OUTPUT) "$(DESTDIR)$(PREFIX)/share/$(OUTPUT)/$(OUTPUT)-$(ARCH)"

uninstall:
	rm -f "$(DESTDIR)$(PREFIX)/share/$(OUTPUT)/$(OUTPUT)-$(ARCH)"
	-rmdir "$(DESTDIR)$(PREFIX)/share/$(OUTPUT)" 2>/dev/null || true