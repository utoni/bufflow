CD := cd
MAKE := make
RM := rm
CC := gcc
STRIP := strip
LBITS := $(shell getconf LONG_BIT)
CFLAGS += -Wall
OCFLAGS += -z execstack -fno-stack-protector
X86_FLAGS = -m32 -mpreferred-stack-boundary=2
X64_FLAGS = -m64 -mpreferred-stack-boundary=4
SOURCES = $(wildcard *.c)
TARGETS = $(patsubst %.c,%,$(SOURCES))

all: $(TARGETS) shellcode crypter

main: $(TARGETS)

debug:
	$(MAKE) -C . CFLAGS="-g"

shellcode:
	$(MAKE) -C shellcode all

crypter:
	$(MAKE) -C crypter all

% : %.c
	$(CC) $(CFLAGS) $(X86_FLAGS) $(OCFLAGS) -o $@ $<
ifeq ($(LBITS),64)
	-$(CC) $(CFLAGS) $(X64_FLAGS) $(OCFLAGS) -o $@_x64 $<
endif

clean:
	$(RM) -f *.o
	$(RM) -f $(TARGETS) $(patsubst %,%_x64,$(TARGETS))
	$(MAKE) -C crypter clean
	$(MAKE) -C shellcode clean

.PHONY: all main shellcode crypter clean
