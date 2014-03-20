RM := rm
CC := gcc
STRIP := strip
CFLAGS = -Wall -g3
OCFLAGS = -m32 -mpreferred-stack-boundary=2 -z execstack -fno-stack-protector
TARGETS = $(patsubst %.c,%.o,$(wildcard *.c))

all: $(TARGETS) post-build

post-build:
	if [ `cat /proc/sys/kernel/randomize_va_space` -eq 0 ]; then \
		echo "not necessary to run ./disable_prot.sh"; \
	else \
		./disable_prot.sh; \
	fi

%.o : %.c
	$(CC) $(CFLAGS) $(OCFLAGS) -o $(patsubst %.o,%,$@) $<
	$(STRIP) $(patsubst %.o,%,$@)

clean:
	$(RM) -f $(patsubst %.o,%,$(TARGETS))

.PHONY: all clean
