BITS 32


jmp		short go
next:
pop		esi
xor		ecx,ecx
xor		eax,eax
mov		cl,0
change:
xor byte	[esi + ecx - 1],0
dec		cl
jnz		change
jmp		short ok
go:
call		next
ok:
