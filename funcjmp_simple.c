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

JUMPABLE_FUNC(testfkt);
JMP_FUNC_DECL(testfkt);

JUMPABLE_FUNC(testfkt)
{
  int var0 = 0x1, var1 = 0x2, var2 = 0x3;
  var0 += var1 + var2;
  return 0;
}

int main(int argc, char **argv)
{
  JMP_TO_FUNC;
  return 66;
}
