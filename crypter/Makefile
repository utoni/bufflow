RM := rm
ASM := nasm
CC := gcc
LD := ld
XXD := xxd
CFLAGS = -c -Wall -fpic -Os
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
ifneq ($(shell if [ -r "$<" ]; then echo "yes"; else echo "no"; fi),yes)
	$(error Necessary file '$<' not found)
endif
	-$(shell ../sc-test -p `cat "$<" | sed -n 's/.*#DECODER=//p'` > $(patsubst %.o,%.h,$@))
	-$(shell ../sc-test -p `cat "$<" | sed -n 's/.*#DECODER=//p'` > $(patsubst %.o,%.h,$@))
	$(CC) $(CFLAGS) -D_USE_CFG -o $@ $<
	$(LD) $(LDFLAGS) $@ -o $(patsubst %.o,%,$@)

clean:
	$(RM) -f $(patsubst %.o,%,$(TARGETS)) $(TARGETS) $(patsubst %.o,%.h,$(TARGETS))

.PHONY: all clean
