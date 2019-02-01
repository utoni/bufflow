CD := cd
LN := ln
MAKE := make
RM := rm
AS := nasm
ifneq ($(BUILD_MINGW32),)
CC := i686-w64-mingw32-gcc
SF := .exe
else
CC ?= gcc
endif
SF ?= $(SUFFIX)
ifneq ($(NASM_FMT32),)
NF32 := $(NASM_FMT32)
else
NF32 := elf32
endif
ifneq ($(NASM_FMT64),)
NF64 := $(NASM_FMT64)
else
NF64 := elf64
endif
STRIP := strip
LBITS := $(shell getconf LONG_BIT)
CFLAGS += -Wall -O0 -g
OCFLAGS := -fno-stack-protector -fno-pie -ggdb -static
ifeq ($(BUILD_MINGW32),)
OCFLAGS += -zexecstack -znorelro
endif
ECFLAGS += -Wall -O2 -ggdb
X86_FLAGS = -m32 -mpreferred-stack-boundary=2
X64_FLAGS = -m64 -mpreferred-stack-boundary=4
SOURCES = $(wildcard *.c)
TARGETS = $(patsubst %.c,%,$(SOURCES))

ifeq ($(LBITS),64)
all: $(TARGETS) exec_payload_x64 exec_crypter_x64 overflow_x64 overflow_tcp_x64 sc-test_x64 shellcode crypter
else
all: $(TARGETS) shellcode crypter
endif

main: $(TARGETS)

exec_payload: exec_payload.c
	$(CC) $(ECFLAGS) -m32 -o $@$(SF) $<
ifneq ($(SF),)
	$(LN) -f -s $@$(SF) $@
endif

exec_payload_x64: exec_payload.c
	$(CC) $(ECFLAGS) -m64 -o $@$(SF) $<

exec_payload_bin.o: exec_payload
	$(STRIP) -s $<$(SF)
	$(AS) -f$(NF32) -o $@ exec_crypter.asm

exec_crypter: exec_payload_bin.o exec_crypter.c
	$(CC) $(ECFLAGS) -m32 -D_NOTASKID=1 -o $@.o -c $@.c
	$(CC) $(ECFLAGS) -m32 -D_NOTASKID=1 -o $@ $(patsubst %.c,%.o,$^)
ifneq ($(SF),)
	$(LN) -f -s $@$(SF) $@
endif

exec_payload_x64_bin.o: exec_payload_x64
	$(STRIP) -s $<
	$(AS) -f$(NF64) -o $@ exec_crypter_x64.asm

exec_crypter_x64: exec_payload_x64_bin.o exec_crypter.c
	$(CC) $(ECFLAGS) -m64 -D_NOTASKID=1 -o $@.o -c exec_crypter.c
	$(CC) $(ECFLAGS) -m64 -D_NOTASKID=1 -o $@ exec_payload_x64_bin.o exec_crypter_x64.o

debug:
	$(MAKE) -C . CFLAGS="-g"

shellcode:
	$(MAKE) -C shellcode all

crypter:
	$(MAKE) -C crypter all

% : %.c
	$(CC) $(CFLAGS) $(X86_FLAGS) $(OCFLAGS) -o $@ $<
%_x64 : %.c
	$(CC) $(CFLAGS) $(X64_FLAGS) $(OCFLAGS) -o $@ $<

rebuild: clean all

clean:
	$(RM) -f *.o
	$(RM) -f $(TARGETS)
ifneq ($(SF),)
	$(RM) -f $(patsubst %,%$(SF),$(TARGETS))
endif
	$(RM) -f exec_payload_x64 exec_crypter_x64 overflow_x64 overflow_tcp_x64 sc-test_x64
ifneq ($(SF),)
	$(RM) -f exec_payload_x64$(SF) exec_crypter_x64$(SF) overflow_x64$(SF) overflow_tcp_x64$(SF) sc-test_x64$(SF)
endif
	$(MAKE) -C crypter clean
	$(MAKE) -C shellcode clean

.PHONY: all main shellcode crypter clean
