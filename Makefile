RM := rm
CC := gcc
STRIP := strip
CFLAGS = -Wall -g3
OCFLAGS = -m32 -mpreferred-stack-boundary=2 -z execstack -fno-stack-protector
TARGETS = $(patsubst %.c,%.o,$(wildcard *.c))

all: $(TARGETS) msg

msg:
	@echo "now run:"
	@echo "  ./disable_prot.sh"
	@echo "  ./exploit"

%.o : %.c
	$(CC) $(CFLAGS) $(OCFLAGS) -o $(patsubst %.o,%,$@) $<
	$(STRIP) $(patsubst %.o,%,$@)

clean:
	$(RM) -f $(patsubst %.o,%,$(TARGETS))

.PHONY: all clean
