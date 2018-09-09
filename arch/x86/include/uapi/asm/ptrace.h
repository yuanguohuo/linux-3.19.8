#ifndef _UAPI_ASM_X86_PTRACE_H
#define _UAPI_ASM_X86_PTRACE_H

#include <linux/compiler.h>	/* For __user */
#include <asm/ptrace-abi.h>
#include <asm/processor-flags.h>


#ifndef __ASSEMBLY__

#ifdef __i386__
/* this struct defines the way the registers are stored on the
   stack during a system call. */

#ifndef __KERNEL__

struct pt_regs {
	long ebx;
	long ecx;
	long edx;
	long esi;
	long edi;
	long ebp;
	long eax;
	int  xds;
	int  xes;
	int  xfs;
	int  xgs;
	long orig_eax;
	long eip;
	int  xcs;
	long eflags;
	long esp;
	int  xss;
};

#endif /* __KERNEL__ */

#else /* __i386__ */

#ifndef __KERNEL__

struct pt_regs {
	unsigned long r15;
	unsigned long r14;
	unsigned long r13;
	unsigned long r12;
	unsigned long rbp;
	unsigned long rbx;
/* arguments: non interrupts/non tracing syscalls only save up to here*/
	unsigned long r11;
	unsigned long r10;
	unsigned long r9;
	unsigned long r8;
	unsigned long rax;
	unsigned long rcx;
	unsigned long rdx;
	unsigned long rsi;
	unsigned long rdi;

  //Yuanguo: for exception: 
  //             if has error code, orig_rax is the error code and it's pushed/saved by hardware (CPU);
  //             else, push 0 by software;
  //         for interupt:
  //             push VectorNo - 256, by software;
	unsigned long orig_rax;

/* end of arguments */
/* cpu exception frame or undefined */
  //Yuanguo: the following are pushed/saved by hardware (CPU), not software;
	unsigned long rip;
	unsigned long cs;
	unsigned long eflags;
	unsigned long rsp;
	unsigned long ss;
/* top of stack page */
};

#endif /* __KERNEL__ */
#endif /* !__i386__ */



#endif /* !__ASSEMBLY__ */

#endif /* _UAPI_ASM_X86_PTRACE_H */
