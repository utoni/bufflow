RM := rm

all: exploit overflow

exploit:
	@echo 'building exploit'
	gcc -Wall -g exploit.c -o exploit

overflow:
	@echo 'building overflow'
	gcc -Wall -m32 -mpreferred-stack-boundary=2 -g -fno-stack-protector overflow.c -o overflow

clean:
	-$(RM) overflow exploit
	-@echo ' '
