RM := rm
CC := gcc
STRIP := strip
LBITS := $(shell getconf LONG_BIT)
CFLAGS = -Wall -g
OCFLAGS = -z execstack -fno-stack-protector
X86_FLAGS = -m32 -mpreferred-stack-boundary=2
X64_FLAGS = -m64 -mpreferred-stack-boundary=4
SOURCES = $(wildcard *.c)
TARGETS = $(patsubst %.c,%.o,$(SOURCES))

all: $(SOURCES) $(TARGETS) shellcode crypter post-build

main: $(SOURCES) $(TARGETS)

shellcode:
	make -f shellcode/Makefile SUBDIR=shellcode

crypter:
	make -f crypter/Makefile SUBDIR=crypter SCDIR=.

post-build:
	@read -p "disable protection stuff? (y/N) " answ; \
	if [ "x$$answ" != "xy" ]; then \
		echo "abort .."; \
		return 0; \
	else \
		./disable_prot.sh; \
	fi

disable-prot:
	if [ `cat /proc/sys/kernel/randomize_va_space` -eq 0 ]; then \
		echo "not necessary to run ./disable_prot.sh"; \
	else \
		./disable_prot.sh; \
	fi

%.o : %.c
	$(CC) $(CFLAGS) $(X86_FLAGS) $(OCFLAGS) -o $(patsubst %.o,%,$@) $<
ifeq ($(LBITS),64)
	-$(CC) $(CFLAGS) $(X64_FLAGS) $(OCFLAGS) -o $(patsubst %.o,%,$@)_x64 $<
endif
	ln -s $< $@

clean:
	$(RM) -f $(patsubst %.o,%,$(TARGETS)) $(patsubst %.c,%_x64,$(wildcard *.c))
	$(RM) -f $(TARGETS)
	make -f shellcode/Makefile SUBDIR=shellcode clean
	make -f crypter/Makefile SUBDIR=crypter clean

.PHONY: shellcode crypter clean
