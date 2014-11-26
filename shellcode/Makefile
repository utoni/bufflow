RM := rm
ASM := nasm
SUBDIR ?= .
TARGETS = $(patsubst %.asm,%.o,$(wildcard $(SUBDIR)/*.asm))

all: $(TARGETS)

%.o : %.asm
	$(ASM) -o $@ $<

clean:
	$(RM) -f $(TARGETS)

.PHONY: all clean
