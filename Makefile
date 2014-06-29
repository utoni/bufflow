RM := rm
CC := gcc
STRIP := strip
CFLAGS = -Wall -g
OCFLAGS = -m32 -mpreferred-stack-boundary=2 -z execstack -fno-stack-protector
TARGETS = $(patsubst %.c,%.o,$(wildcard *.c))

all: $(TARGETS) post-build

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
	$(CC) $(CFLAGS) $(OCFLAGS) -o $(patsubst %.o,%,$@) $<

clean:
	$(RM) -f $(patsubst %.o,%,$(TARGETS))

.PHONY: all clean
