RM := rm
ASM := nasm
CC := gcc
LD := ld
XXD := xxd
ASMFLAGS = -g
CFLAGS += -fpic -Os
X86_FLAGS := -m32
X64_FLAGS := -m64
LDFLAGS ?=
SUBDIR ?= .
TARGETS := $(patsubst %.asm,%.o,$(wildcard $(SUBDIR)/*.asm)) $(patsubst %.c,%,$(wildcard $(SUBDIR)/*.c))
SOURCES_ASM := $(wildcard $(SUBDIR)/*.asm)
SOURCES_C := $(wildcard $(SUBDIR)/*.c)
SCDIR ?= ..
SCC := $(shell if [ -x $(SCDIR)/sc-test ]; then echo "yes"; else echo "no"; fi)
LBITS := $(shell getconf LONG_BIT)


all: $(TARGETS)

%.o : %.asm
	$(ASM) $(ASMFLAGS) -o $@ $<

% : %.c
ifneq ($(SCC),yes)
	$(error Please run 'make' first in the main directory)
endif
	@echo "generating header $(patsubst %.c,%.h,$<) for target $<"
	-$(shell $(SCDIR)/sc-test -p $(SUBDIR)/`cat "$<" | sed -n 's/.*#DECODER=//p'` | sed 's/shellcode/decoder/' > $(patsubst %.c,%.h,$<))
	-$(shell $(SCDIR)/sc-test -p $(SUBDIR)/`cat "$<" | sed -n 's/.*#SHELLCODE=//p'` >> $(patsubst %.c,%.h,$<))
	$(CC) $(CFLAGS) $(X86_FLAGS) -D_USE_CFG -o $(patsubst %.c,%,$<) $<
ifeq ($(LBITS),64)
	$(CC) $(CFLAGS) $(X64_FLAGS) -D_USE_CFG -o $(patsubst %.c,%_x64,$<) $<
endif

clean:
	$(RM) -f $(patsubst %.c,%,$(SOURCES_C)) $(patsubst %.c,%_x64,$(SOURCES_C))
	$(RM) -f $(patsubst %.c,%.h,$(SOURCES_C))
	$(RM) -f $(patsubst %.asm,%.o,$(SOURCES_ASM))

.PHONY: all clean
