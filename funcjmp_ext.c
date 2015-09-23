#include <stdio.h>
#include <stdlib.h>


#define INTEL_ASM(_asm_str) asm volatile(".intel_syntax noprefix"); \
	asm volatile(_asm_str); \
	asm volatile(".att_syntax prefix");
#define JUMPABLE_FUNC(fname) __attribute__ ((__cdecl__)) int fname(void)
#define JMP_FUNC_DECL(func) void *fptr = (void *)( &func );
#define JMP_TO_FUNC \
	INTEL_ASM(" \
	        call getip; \
	        jmp short donext; \
	        cfunc: \
	                mov eax,[fptr]; \
	                add eax,0x0; \
	                jmp eax; \
	                ret; \
	        getip: \
	                nop; \
	                jmp short cfunc; \
	        donext: \
	");

#define PRE_JUMP(arg)
		

int hookable(char *arg0, int arg1, int arg2)
{
  asm("label:");
  INTEL_ASM("nop; nop; nop");
  asm("jmp end");
  return 0;
}

int main(int argc, char **argv)
{
  asm("push %0" : : "m" (hookable));
  asm("push %0" : : "g" (hookable));
  asm("jmp label; \
	end:");
  //hookable(NULL, 0x8, 0x9);
  return 66;
}

