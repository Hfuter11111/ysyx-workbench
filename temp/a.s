	.file	"a.c"
	.option nopic
	.attribute arch, "rv32i2p1_m2p0"
	.attribute unaligned_access, 0
	.attribute stack_align, 16
	.text
	.align	2
	.globl	f1
	.type	f1, @function
f1:
	sgt	a0,a0,a1
	ret
	.size	f1, .-f1
	.align	2
	.globl	f2
	.type	f2, @function
f2:
	slt	a0,a0,a1
	ret
	.size	f2, .-f2
	.align	2
	.globl	f3
	.type	f3, @function
f3:
	slt	a0,a0,a1
	seqz	a0,a0
	ret
	.size	f3, .-f3
	.align	2
	.globl	f4
	.type	f4, @function
f4:
	sgt	a0,a0,a1
	seqz	a0,a0
	ret
	.size	f4, .-f4
	.ident	"GCC: (13.2.0-11ubuntu1+12) 13.2.0"
