BITS 32

xor		eax,eax
xor		ebx,ebx
xor		ecx,ecx
xor		edx,edx
jmp short string
code:
pop		ecx
mov		bl,1
mov		dl,12
mov		al,4
int		0x80
dec		bl
mov		al,1
int		0x80
string:
call code
db 'Hello World!'
