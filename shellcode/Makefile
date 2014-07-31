RM := rm
CC := gcc
LD := ld
OBJCOPY := objcopy
XXD := xxd
CFLAGS = -c -Wall -fpic -Os
LDFLAGS =
SUBDIR ?= .
TARGETS = $(patsubst %.c,%.o,$(wildcard $(SUBDIR)/*.c))

all: $(TARGETS)

%.o : %.c
	$(CC) $(CFLAGS) -o $@ $<
	$(LD) $(LDFLAGS) -N -Ttext 0x0 -e _start -Map $(patsubst %.o,%.map,$@) $@ -o $(patsubst %.o,%,$@)
	$(OBJCOPY) -R .note -R .comment -S -O binary $(patsubst %.o,%,$@) $(patsubst %.o,%.bin,$@)
	$(XXD) -i $(patsubst %.o,%.bin,$@)

clean:
	$(RM) -f $(patsubst %.o,%.map,$(TARGETS)) $(patsubst %.o,%.bin,$(TARGETS)) $(patsubst %.o,%,$(TARGETS)) $(TARGETS)

.PHONY: all clean
