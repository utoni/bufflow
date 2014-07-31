/*
 * gcc -c -Wall -fpic -Os shellcode.c -o shellcode.o
 * ld -N -Ttext 0x0 -e _start -Map shellcode.map shellcode.o -o shellcode
 * objcopy -R .note -R .comment -S -O binary shellcode shellcode.bin
 */

int _start(void) {
  while (1) {
  }
  return (0);
}
