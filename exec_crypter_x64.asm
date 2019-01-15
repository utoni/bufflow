bits 64
section .rodata

global _exec_payload_start
global _exec_payload_end
global _exec_payload_size

db 0xde,0xad,0xc0,0xde ; marker
dd 0x00000000,0x00000000,0x00000000,0x00000000 ; xor key
_exec_payload_start: incbin "exec_payload_x64"
_exec_payload_end:
_exec_payload_size:  dd $ - _exec_payload_start
