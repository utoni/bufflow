CD := cd
MAKE := make
RM := rm
AS := nasm
CC := gcc
STRIP := strip
LBITS := $(shell getconf LONG_BIT)
CFLAGS += -Wall -O0 -g
OCFLAGS += -zexecstack -znorelro -fno-stack-protector -fno-pie -ggdb -static
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
	$(CC) $(ECFLAGS) -m32 -o $@ $<

exec_payload_x64: exec_payload.c
	$(CC) $(ECFLAGS) -m64 -o $@ $<

exec_payload_bin.o: exec_payload
	$(STRIP) -s $<
	$(AS) -felf32 -o $@ exec_crypter.asm

exec_crypter: exec_payload_bin.o exec_crypter.c
	$(CC) $(ECFLAGS) -m32 -D_NOTASKID=1 -o $@.o -c $@.c
	$(CC) $(ECFLAGS) -m32 -D_NOTASKID=1 -o $@ $(patsubst %.c,%.o,$^)

exec_payload_x64_bin.o: exec_payload_x64
	$(STRIP) -s $<
	$(AS) -felf64 -o $@ exec_crypter_x64.asm

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
	$(RM) -f $(TARGETS) $(patsubst %,%_x64,$(TARGETS))
	$(RM) -f exec_payload_x64 exec_crypter_x64 overflow_x64 overflow_tcp_x64 sc-test_x64
	$(MAKE) -C crypter clean
	$(MAKE) -C shellcode clean

.PHONY: all main shellcode crypter clean
