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

; ...
