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

	.globl	func
	.p2align	4, 0x90
	.type	func,@function
func:
	.cfi_startproc
	movq	%rdi, -8(%rsp)
	movl	$100, 0
	retq
.Lfunc_end1:
	.size	func, .Lfunc_end1-func
	.cfi_endproc

	.section	.rodata.cst16,"aM",@progbits,16
	.p2align	4, 0x0
.LCPI2_0:
	.long	1
	.long	2
	.long	3
	.long	4
	.text
	.globl	main
	.p2align	4, 0x90
	.type	main,@function
main:
	.cfi_startproc
	subq	$24, %rsp
	.cfi_def_cfa_offset 32
	movaps	.LCPI2_0(%rip), %xmm0
	movups	%xmm0, 8(%rsp)
	leaq	8(%rsp), %rdi
	callq	func@PLT
	movl	16(%rsp), %esi
	movl	$.Lfmt.1, %edi
	xorl	%eax, %eax
	callq	printf@PLT
	xorl	%eax, %eax
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

	.type	.Lfmt.1,@object
.Lfmt.1:
	.asciz	"\"%d\\n\""
	.size	.Lfmt.1, 7

	.section	".note.GNU-stack","",@progbits
