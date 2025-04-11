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
	subq	$24, %rsp
	.cfi_def_cfa_offset 32
	movl	$0, 12(%rsp)
	callq	getint@PLT
	movl	%eax, 12(%rsp)
	movl	$4, 20(%rsp)
	addl	$4, %eax
	movl	%eax, 16(%rsp)
	cmpl	$3, %eax
	jl	.LBB1_3
	movl	$.Lfmt.1, %edi
	jmp	.LBB1_2
.LBB1_3:
	movl	$.Lfmt.2, %edi
.LBB1_2:
	xorl	%eax, %eax
	callq	printf@PLT
	movl	12(%rsp), %eax
	addq	$24, %rsp
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

	.type	.Lfmt.1,@object
.Lfmt.1:
	.asciz	"\"true\""
	.size	.Lfmt.1, 7

	.type	.Lfmt.2,@object
.Lfmt.2:
	.asciz	"\"false\""
	.size	.Lfmt.2, 8

	.section	".note.GNU-stack","",@progbits
