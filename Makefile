RM := rm

all: exploit overflow

exploit:
	@echo 'building exploit'
	gcc -Wall -g exploit.c -o exploit

overflow:
	@echo 'building overflow'
	gcc -Wall -g -fno-stack-protector --param ssp-buffer-size=2 overflow.c -o overflow

clean:
	-$(RM) overflow exploit
	-@echo ' '
