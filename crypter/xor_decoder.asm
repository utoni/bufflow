BITS 32


jmp		short go
next:
pop		esi
xor		ecx,ecx
xor		eax,eax
mov		cl,0
change:
mov byte	al,0
xor byte	al,[esi + ecx - 1]

dec		cl
jnz		change
jmp		short ok
go:
call		next
ok:
