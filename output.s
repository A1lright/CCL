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

	.globl	add
	.p2align	4, 0x90
	.type	add,@function
add:
	.cfi_startproc
	movl	%edi, -4(%rsp)
	movl	%esi, -8(%rsp)
	leal	(%rdi,%rsi), %eax
	movl	%eax, -12(%rsp)
	retq
.Lfunc_end1:
	.size	add, .Lfunc_end1-add
	.cfi_endproc

	.globl	main
	.p2align	4, 0x90
	.type	main,@function
main:
	.cfi_startproc
	subq	$24, %rsp
	.cfi_def_cfa_offset 32
	movl	$1, 20(%rsp)
	movl	$2, 16(%rsp)
	movl	$0, 12(%rsp)
	movl	$1, %edi
	movl	$2, %esi
	callq	add@PLT
	movl	%eax, 12(%rsp)
	addq	$24, %rsp
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end2:
	.size	main, .Lfunc_end2-main
	.cfi_endproc

	.type	.Lfmt,@object
	.section	.rodata.str1.1,"aMS",@progbits,1
.Lfmt:
	.asciz	"%d"
	.size	.Lfmt, 3

	.section	".note.GNU-stack","",@progbits
