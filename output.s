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
	subq	$40, %rsp
	.cfi_def_cfa_offset 48
	movabsq	$8589934592, %rax
	movq	%rax, 4(%rsp)
	movl	$1, 4(%rsp)
	movl	$3, 12(%rsp)
	movabsq	$21474836484, %rax
	movq	%rax, 16(%rsp)
	movabsq	$30064771078, %rax
	movq	%rax, 24(%rsp)
	movabsq	$38654705672, %rax
	movq	%rax, 32(%rsp)
	movl	$.Lfmt.1, %edi
	movl	$1, %esi
	movl	$2, %edx
	movl	$3, %ecx
	xorl	%eax, %eax
	callq	printf@PLT
	movl	16(%rsp), %esi
	movl	20(%rsp), %edx
	movl	24(%rsp), %ecx
	movl	$.Lfmt.2, %edi
	xorl	%eax, %eax
	callq	printf@PLT
	movl	28(%rsp), %esi
	movl	32(%rsp), %edx
	movl	36(%rsp), %ecx
	movl	$.Lfmt.3, %edi
	xorl	%eax, %eax
	callq	printf@PLT
	xorl	%eax, %eax
	addq	$40, %rsp
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
	.asciz	"\"%d %d %d\\n\""
	.size	.Lfmt.1, 13

	.type	.Lfmt.2,@object
.Lfmt.2:
	.asciz	"\"%d %d %d\\n\""
	.size	.Lfmt.2, 13

	.type	.Lfmt.3,@object
.Lfmt.3:
	.asciz	"\"%d %d %d\\n\""
	.size	.Lfmt.3, 13

	.section	".note.GNU-stack","",@progbits
