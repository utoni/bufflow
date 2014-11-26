RM := rm
ASM := nasm
CC := gcc
LD := ld
XXD := xxd
CFLAGS ?= -Wall -fpic -Os
X86_FLAGS := -m32
X64_FLAGS := -m64
LDFLAGS ?=
SUBDIR ?= .
TARGETS := $(patsubst %.asm,%.o,$(wildcard $(SUBDIR)/*.asm)) $(patsubst %.c,%,$(wildcard $(SUBDIR)/*.c))
SOURCES_ASM := $(wildcard $(SUBDIR)/*.asm)
SOURCES_C := $(wildcard $(SUBDIR)/*.c)
SCDIR ?= ..
SCC := $(shell if [ -x $(SCDIR)/sc-test ]; then echo "yes"; else echo "no"; fi)


all: $(TARGETS)

%.o : %.asm
	$(ASM) -o $@ $<

% : %.c
ifneq ($(SCC),yes)
	$(error Please run 'make' first in the main directory)
endif
	-$(shell $(SCDIR)/sc-test -p $(SUBDIR)/`cat "$<" | sed -n 's/.*#DECODER=//p'` | sed 's/shellcode/decoder/' > $(patsubst %.o,%.h,$@))
	-$(shell $(SCDIR)/sc-test -p $(SUBDIR)/`cat "$<" | sed -n 's/.*#SHELLCODE=//p'` >> $(patsubst %.o,%.h,$@))
	$(CC) $(CFLAGS) $(X86_FLAGS) -D_USE_CFG -o $(patsubst %.c,%,$<) $<
	$(CC) $(CFLAGS) $(X64_FLAGS) -D_USE_CFG -o $(patsubst %.c,%_x64,$<) $<

clean:
	$(RM) -f $(patsubst %.c,%,$(SOURCES_C)) $(patsubst %.c,%_x64,$(SOURCES_C))
	$(RM) -f $(patsubst %.asm,%.o,$(SOURCES_ASM))

.PHONY: all clean
