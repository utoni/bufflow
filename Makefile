RM := rm
CC := gcc
CFLAGS = -Wall -g3
OCFLAGS = -m32 -mpreferred-stack-boundary=2 -z execstack -fno-stack-protector
BINS = exploit
OBINS = overflow overflow_minimal overflow_function exploit

all: exploit overflow

exploit:
	@echo 'building exploits'
	for file in $(BINS); do \
		$(CC) $(CFLAGS) exploit.c -o exploit; \
	done

overflow:
	@echo 'building exploitable binaries'
	for file in $(OBINS); do \
		$(CC) $(CFLAGS) $(OCFLAGS) $$file.c -o $$file; \
	done

clean:
	for file in $(BINS); do \
		$(RM) -f $$file; \
	done
	for file in $(OBINS); do \
		$(RM) -f $$file; \
	done
	@echo ' '

.PHONY: all clean
