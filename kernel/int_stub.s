import kernel_enter
import _kernel_exit

MACRO HANDLER
export _intr_?1
_intr_?1:
	call kernel_enter

	mov ax, #?1
	push ax
	jmp word intr_common
MEND

; for exceptions
HANDLER 0
HANDLER 1
HANDLER 2
HANDLER 3
HANDLER 4
HANDLER 5
HANDLER 6
HANDLER 7

;for irqs 0-7
HANDLER 8
HANDLER 9
HANDLER 10
HANDLER 11
HANDLER 12
HANDLER 13
HANDLER 14
HANDLER 15

; for irqs 0x70-0x77
HANDLER 16
HANDLER 17
HANDLER 18
HANDLER 19
HANDLER 20
HANDLER 21
HANDLER 22
HANDLER 23

;for syscall
HANDLER 24

export _intr_quick
_intr_quick:
	iret

import _intr_dispatcher

intr_common:
	call _intr_dispatcher
	pop ax
	jmp word _kernel_exit

