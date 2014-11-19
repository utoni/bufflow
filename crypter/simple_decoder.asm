BITS 32


jmp		short go
next:
pop		esi
xor		ecx,ecx
mov		cl,0
change:
sub byte	[esi + ecx - 1],0
dec		cl
jnz		change
jmp		short ok
go:
call		next
ok:
