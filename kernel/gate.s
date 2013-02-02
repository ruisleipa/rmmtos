get stack.s

import _current_task

export kernel_enter
kernel_enter:
	; The return address pushed to the stack by the call is left intact.
	push ax
	push bx
	push cx
	push dx
	push si
	push di
	push bp

	; get the return address from stack
	mov bp, sp
	mov cx, 14[bp]

	mov ax, ss
	cmp ax, #0x0050
	je .return
	; entering from task
	; save the stack pointer in the task struct
	mov ax, #0x0050
	mov ds, ax
	mov es, ax

	mov bx, [_current_task]
	mov [bx], bp

	mov dx, 2[bx]
	mov ss, ax
	mov sp, dx
.return:

	push ax

	mov ax, ds
	cmp ax, #0x0050
.loop:
	jne .loop

	pop ax
	push cx
	ret

export kernel_exit_force
kernel_exit_force:
	mov word bp, [_current_task]
	mov word ax, [bp]
	mov word bx, 4[bp]

	mov ss, bx
	mov sp, ax

	mov ds, bx
	mov es, bx

	pop bp
	pop di
	pop si
	pop dx
	pop cx
	pop bx
	pop ax

	add sp, #2

	iret

export _kernel_exit
_kernel_exit:
	mov word bp, [_current_task]
	mov word ax, [bp]
	mov word bx, 4[bp]
	mov word cx, 2[bp]
	mov word dx, 6[bp]

	;check if the kernel stack is empty
	cmp sp, cx
	;if it isnt, don't switch to task stack
	jne .kernel

	cli

	mov ss, bx
	mov sp, ax

	mov ds, bx
	mov es, bx
.kernel:
	pop bp
	pop di
	pop si
	pop dx
	pop cx
	pop bx
	pop ax

	; remove return address of kernel_enter from the stack
	add sp, #2

	iret


