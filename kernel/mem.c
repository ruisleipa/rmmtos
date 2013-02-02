void pokew(unsigned int seg,unsigned int off,unsigned int value)
{
#asm
	push bp
	mov  bp, sp

	push ax
	push bx
	push cx
	pushf

	cli

	mov  ax, 4[bp] ; segment
	mov  cx, ds
	mov  ds, ax

	mov  bx, 6[bp] ; offset
	mov  ax, 8[bp] ; data word
	mov  [bx], ax  ; write data word

	mov ds, cx

	popf
	pop  cx
	pop  bx
	pop  ax

	pop  bp
#endasm

}

void poke(unsigned int seg,unsigned int off,char value)
{
#asm
	push bp
	mov  bp, sp

	push ax
	push bx
	push cx
	pushf

	cli

	mov ax, 4[bp] ; segment
	mov cx, ds
	mov ds, ax

	mov bx, 6[bp] ; offset
	mov al, 8[bp] ; data byte
	mov [bx], al  ; write data byte

	mov ds, cx

	popf

	pop cx
	pop bx
	pop ax

	pop bp
#endasm

}

void peekw(unsigned int seg,unsigned int off,unsigned int* value)
{
#asm
	push bp
	mov bp, sp

	push ax
	push bx
	push cx
	pushf

	cli

	mov ax, 4[bp]
	mov cx, ds
	mov ds, ax

	mov bx, 6[bp]
	mov ax, [bx]

	mov ds, cx

	mov bx, 8[bp]
	mov [bx], ax

	popf
	pop cx
	pop bx
	pop ax

	pop bp
#endasm

}

void peek(unsigned int seg,unsigned int off,char* value)
{
#asm
	push bp
	mov bp, sp

	push ax
	push bx
	push cx
	pushf

	cli

	mov ax, 4[bp]
	mov cx, ds
	mov ds, ax

	mov bx, 6[bp]
	mov al, [bx]

	mov ds, cx

	mov bx, 8[bp]
	mov [bx], al

	popf
	pop cx
	pop bx
	pop ax

	pop bp
#endasm

}

