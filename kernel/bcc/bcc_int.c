/************************************************************************/
/* This file contains the BCC compiler helper functions */
/* (C) Copyright Bruce Evans */
/* Support for integer arithmetic */

#ifdef __AS386_16__
#asm
	.text	! This is common to all.
	.even
#endasm

/************************************************************************/
/* Function idiv */

#asm

! idiv.s
! idiv_ doesn`t preserve dx (returns remainder in it)

	.globl idiv_

idiv_:
	cwd
	idiv	bx
	ret
#endasm

/************************************************************************/
/* Function idivu */

#asm

! idivu.s
! idiv_u doesn`t preserve dx (returns remainder in it)

	.globl idiv_u

idiv_u:
	xor	dx,dx
	div	bx
	ret
#endasm

/************************************************************************/
/* Function imod */

#asm

! imod.s
! imod doesn`t preserve dx (returns quotient in it)

	.globl imod

imod:
	cwd
	idiv	bx
	mov	ax,dx
	ret
#endasm

/************************************************************************/
/* Function imodu */

#asm

! imodu.s
! imodu doesn`t preserve dx (returns quotient in it)

	.globl imodu

imodu:
	xor	dx,dx
	div	bx
	mov	ax,dx		! instruction queue full so xchg slower
	ret
#endasm

/************************************************************************/
/* Function imul */

#asm

! imul.s
! imul_, imul_u don`t preserve dx

	.globl imul_
	.globl imul_u

imul_:
imul_u:
	imul	bx
	ret
#endasm

/************************************************************************/
/* Function isl */

#asm

! isl.s
! isl, islu don`t preserve cl

	.globl isl
	.globl islu

isl:
islu:
	mov	cl,bl
	shl	ax,cl
	ret
#endasm

/************************************************************************/
/* Function isr */

#asm

! isr.s
! isr doesn`t preserve cl

	.globl isr

isr:
	mov	cl,bl
	sar	ax,cl
	ret
#endasm

/************************************************************************/
/* Function isru */

#asm

! isru.s
! isru doesn`t preserve cl

	.globl isru

isru:
	mov	cl,bl
	shr	ax,cl
	ret
#endasm

