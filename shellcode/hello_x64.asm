BITS 64

call get_string
got_string:
pop		rsi
xor		rax,rax
xor		rdx,rdx
mov byte	al,0x1
mov byte	dl,0x7
syscall
jmp short	done

get_string:
jmp short	got_string
db		'blablubb'
done:
