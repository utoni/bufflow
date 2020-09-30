#include <stdio.h>
#include <stdlib.h>

#ifndef __i386
#error "Unsupported Architecture"
#endif

#define INTEL_ASM(_asm_str) asm volatile(".intel_syntax noprefix"); \
	asm volatile(_asm_str); \
	asm volatile(".att_syntax prefix");

int hookable(char *arg0, int arg1, int arg2)
{
  asm("label:");
  INTEL_ASM("nop; nop; nop");
  printf("hookable ..\n");
  asm("nop; nop; nop; pop %ebx; pop %eax; call *%eax; call *%ebx");
  asm("jmp end");
  return 0;
}

int testfkt(void)
{
  printf("Subroutine ..\n");
  return 0;
}

void testfkt2(void)
{
  printf("another Subroutine ..\n");
}

int main(int argc, char **argv)
{
  printf("main(...)\n");
  asm("push %0" : : "g" (testfkt));
  asm("push %0" : : "g" (testfkt2));
  asm("jmp label; \
	end:");
  printf("EOF!\n");
  return 0;
}

