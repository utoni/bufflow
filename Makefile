RM := rm

all: exploit overflow

exploit:
	@echo 'building exploit'
	gcc -g -fno-stack-protector exploit.c -o exploit

overflow:
	@echo 'building overflow'
	gcc -g -fno-stack-protector overflow.c -o overflow

clean:
	-$(RM) overflow exploit
	-@echo ' '
