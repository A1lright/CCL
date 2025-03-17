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
	pushq	%rax
	.cfi_def_cfa_offset 16
	callq	getint@PLT
	movl	%eax, 4(%rsp)
	movl	$.Lfmt.1, %edi
	movl	%eax, %esi
	xorl	%eax, %eax
	callq	printf@PLT
	movl	4(%rsp), %eax
	popq	%rcx
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
	.asciz	"\"%d\""
	.size	.Lfmt.1, 5

	.section	".note.GNU-stack","",@progbits
