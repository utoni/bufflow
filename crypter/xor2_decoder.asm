BITS 32


jmp		short go
next:
pop		esi
xor		ecx,ecx
xor		eax,eax
xor		edx,edx
mov		cl,0		; buffer length
mov		dl,4		; xor padding
change:
xor byte	[esi + ecx - 1],0

dec		cl
jnz		done		; no more bytes left
dec		dh
jnz		change
mov		dh,dl
jmp		change

done:
jmp		short ok
go:
call		next
ok:
