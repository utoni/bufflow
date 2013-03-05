RM := rm

all: exploit overflow

exploit:
	@echo 'building exploit'
	gcc -Wall -g exploit.c -o exploit

overflow:
	@echo 'building overflow'
	gcc -Wall -m32 -mpreferred-stack-boundary=2 -g -fno-stack-protector overflow.c -o overflow

test: overflow
	@if [ -x /usr/bin/python ]; then \
	  ./overflow `python -c 'print "A"*5000'`; \
	else \
	  echo 'Missing PYTHON; not testing'; \
	fi
	@echo 'TEST FAILED: ./overflow not segfaulting'

clean:
	-$(RM) -f overflow exploit
	-@echo ' '

.PHONY: all clean
