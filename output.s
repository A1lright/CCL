	.text
	.file	"SysY_module"
	.globl	getint
	.p2align	4, 0x90
	.type	getint,@function
getint:
	.cfi_startproc
	pushq	%rax
	.cfi_def_cfa_offset 16
	leaq	4(%rsp), %rsi
	movl	$.Lfmt, %edi
	xorl	%eax, %eax
	callq	scanf@PLT
	movl	4(%rsp), %eax
	popq	%rcx
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end0:
	.size	getint, .Lfunc_end0-getint
	.cfi_endproc

	.globl	main
	.p2align	4, 0x90
	.type	main,@function
main:
	.cfi_startproc
	pushq	%rbx
	.cfi_def_cfa_offset 16
	.cfi_offset %rbx, -16
	callq	getint@PLT
	movq	c@GOTPCREL(%rip), %rbx
	movl	%eax, (%rbx)
	movl	$.Lfmt.1, %edi
	movl	%eax, %esi
	xorl	%eax, %eax
	callq	printf@PLT
	movl	(%rbx), %eax
	popq	%rbx
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end1:
	.size	main, .Lfunc_end1-main
	.cfi_endproc

	.type	.Lfmt,@object
	.section	.rodata.str1.1,"aMS",@progbits,1
.Lfmt:
	.asciz	"%d"
	.size	.Lfmt, 3

	.type	c,@object
	.bss
	.globl	c
	.p2align	2, 0x0
c:
	.long	0
	.size	c, 4

	.type	.Lfmt.1,@object
	.section	.rodata.str1.1,"aMS",@progbits,1
.Lfmt.1:
	.asciz	"\"%d\""
	.size	.Lfmt.1, 5

	.section	".note.GNU-stack","",@progbits
