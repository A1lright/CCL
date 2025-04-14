	.text
	.file	"SysY_module"
	.globl	getint                          # -- Begin function getint
	.p2align	4, 0x90
	.type	getint,@function
getint:                                 # @getint
	.cfi_startproc
# %bb.0:                                # %entry
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
                                        # -- End function
	.globl	func                            # -- Begin function func
	.p2align	4, 0x90
	.type	func,@function
func:                                   # @func
	.cfi_startproc
# %bb.0:                                # %entry
	movq	%rdi, -8(%rsp)
	movl	$100, (%rsp)
	retq
.Lfunc_end1:
	.size	func, .Lfunc_end1-func
	.cfi_endproc
                                        # -- End function
	.globl	main                            # -- Begin function main
	.p2align	4, 0x90
	.type	main,@function
main:                                   # @main
	.cfi_startproc
# %bb.0:                                # %entry
	subq	$24, %rsp
	.cfi_def_cfa_offset 32
	movabsq	$8589934593, %rax               # imm = 0x200000001
	movq	%rax, 8(%rsp)
	movabsq	$17179869187, %rax              # imm = 0x400000003
	movq	%rax, 16(%rsp)
	leaq	8(%rsp), %rdi
	callq	func@PLT
	movl	24(%rsp), %esi
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
                                        # -- End function
	.type	.Lfmt,@object                   # @fmt
	.section	.rodata.str1.1,"aMS",@progbits,1
.Lfmt:
	.asciz	"%d"
	.size	.Lfmt, 3

	.type	.Lfmt.1,@object                 # @fmt.1
.Lfmt.1:
	.asciz	"\"%d\\n\""
	.size	.Lfmt.1, 7

	.section	".note.GNU-stack","",@progbits
