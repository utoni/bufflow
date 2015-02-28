BITS 32


jmp		short go
next:
pop		esi		; stackpointer -> start+len(encoder)
xor		ecx,ecx		; zero out some regs
xor		eax,eax
xor		edx,edx
mov		cl,0		; buffer length
mov		dl,4		; xor padding
change:
xor byte	[esi + ecx],0
mov byte	al,[esi + ecx]
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
