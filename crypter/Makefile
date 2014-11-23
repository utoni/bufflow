RM := rm
ASM := nasm
CC := gcc
LD := ld
XXD := xxd
CFLAGS = -Wall -fpic -Os
LDFLAGS =
SUBDIR ?= .
TARGETS = $(patsubst %.asm,%.o,$(wildcard $(SUBDIR)/*.asm)) $(patsubst %.c,%.o,$(wildcard $(SUBDIR)/*.c))
SCDIR ?= ..
SCC := $(shell if [ -x $(SCDIR)/sc-test ]; then echo "yes"; else echo "no"; fi)


all: $(TARGETS)

%.o : %.asm
	$(ASM) -o $@ $<

%.o : %.c
ifneq ($(SCC),yes)
	$(error Please run 'make' in the main directory)
endif
	-$(shell $(SCDIR)/sc-test -p $(SUBDIR)/`cat "$<" | sed -n 's/.*#DECODER=//p'` | sed 's/shellcode/decoder/' > $(patsubst %.o,%.h,$@))
	-$(shell $(SCDIR)/sc-test -p $(SUBDIR)/`cat "$<" | sed -n 's/.*#SHELLCODE=//p'` >> $(patsubst %.o,%.h,$@))
	$(CC) $(CFLAGS) -D_USE_CFG -o $(patsubst %.c,%,$<) $<

clean:
	$(RM) -f $(patsubst %.o,%,$(TARGETS)) $(TARGETS) $(patsubst %.o,%.h,$(TARGETS))
	$(RM) -f *.o

.PHONY: all clean
