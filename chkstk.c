/* SPDX-License-Identifier: GPL-2.0-or-later */

#if defined(__GNUC__) && defined(_NOCRT)
#ifdef __x86_64__
__asm__(
	".global ___chkstk_ms\n"
	"___chkstk_ms:\n"
	"	push   %rcx\n"
	"	push   %rax\n"
	"	cmp	$0x1000,%rax\n"
	"	lea	24(%rsp),%rcx\n"
	"	jb	 done\n"
	"loop:\n"
	"	sub	$0x1000,%rcx\n"
	"	testl  $0,(%rcx)\n"
	"	sub	$0x1000,%rax\n"
	"	cmp	$0x1000,%rax\n"
	"	ja	 loop\n"
	"done:\n"
	"	sub	%rax,%rcx\n"
	"	testl  $0,(%rcx)\n"
	"	pop	%rax\n"
	"	pop	%rcx\n"
	"	ret\n"
);
#elif defined(__i386__)
__asm__(
	".global ___chkstk_ms\n"
	"___chkstk_ms:\n"
	"	push   %ecx\n"
	"	push   %eax\n"
	"	push   %edx\n"
	"	cmp	$0x1000,%eax\n"
	"	lea	16(%esp),%ecx\n"
	"	jb	 done\n"
	"loop:\n"
	"	sub	$0x1000,%ecx\n"
	"	movl   (%ecx),%edx\n"
	"	movl   %edx,(%ecx)\n"
	"	sub	$0x1000,%eax\n"
	"	cmp	$0x1000,%eax\n"
	"	ja	 loop\n"
	"done:\n"
	"	sub	%eax,%ecx\n"
	"	movl   (%ecx),%edx\n"
	"	movl   %edx,(%ecx)\n"
	"	pop	%edx\n"
	"	pop	%eax\n"
	"	pop	%ecx\n"
	"	ret\n"
);
#endif
#endif
