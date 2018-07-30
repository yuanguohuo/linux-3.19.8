#ifndef _ASM_X86_RMWcc
#define _ASM_X86_RMWcc

#ifdef CC_HAVE_ASM_GOTO

#define __GEN_RMWcc(fullop, var, cc, ...)				\
do {									\
	asm_volatile_goto (fullop "; j" cc " %l[cc_label]"		\
			: : "m" (var), ## __VA_ARGS__ 			\
			: "memory" : cc_label);				\
	return 0;							\
cc_label:								\
	return 1;							\
} while (0)


/*
 * Yuanguo: example:
 * GEN_UNARY_RMWcc(LOCK_PREFIX "decq", v->counter, "%0", "e");  ==>
 * GEN_UNARY_RMWcc("...\n\tlock; decq", v->counter, "%0", "e");  ==>
 * __GEN_RMWcc("...\n\tlock; decq %0", v->counter, "e");  ==>
 *
 *  do {
 *  	asm_volatile_goto ( "...lock; decq %0; je %l[cc_label]" //Yuanguo: if --(v->counter)==0, jump to cc_label
 *  			: : "m" (v->counter), ## __VA_ARGS__ 
 *  			: "memory" : cc_label);
 *  	return 0;							
 *  cc_label:	
 *  	return 1; //Yuanguo: return true if --(v->counter)==0
 *  } while (0);
 */

#define GEN_UNARY_RMWcc(op, var, arg0, cc) 				\
	__GEN_RMWcc(op " " arg0, var, cc)

#define GEN_BINARY_RMWcc(op, var, vcon, val, arg0, cc)			\
	__GEN_RMWcc(op " %1, " arg0, var, cc, vcon (val))

#else /* !CC_HAVE_ASM_GOTO */

#define __GEN_RMWcc(fullop, var, cc, ...)				\
do {									\
	char c;								\
	asm volatile (fullop "; set" cc " %1"				\
			: "+m" (var), "=qm" (c)				\
			: __VA_ARGS__ : "memory");			\
	return c != 0;							\
} while (0)


/*
 * Yuanguo: example
 * GEN_UNARY_RMWcc(LOCK_PREFIX "decq", v->counter, "%0", "e");  ==>
 * GEN_UNARY_RMWcc("...\n\tlock; decq", v->counter, "%0", "e");  ==>
 * __GEN_RMWcc("...\n\tlock; decq %0", v->counter, "0"); =>
 *
 *  do {
 *  	char c;
 *  	asm volatile ("...\n\tlock; decq %0; sete %1" //Yuanguo: c(%1)=ZF, that's: if --(v->counter)==0, then c=1
 *  			: "+m" (var), "=qm" (c)			
 *  			: __VA_ARGS__ : "memory");	
 *  	return c != 0;		//Yuanguo: return true if c==1, that's, return true if --(v->counter)==0
 *  } while (0);
 *
 * Yuanguo: instruct 'sete' is the same as instruction 'setz',
 *     sete param   ---> param = ZF 
 *     setz param   ---> param = ZF 
 */

#define GEN_UNARY_RMWcc(op, var, arg0, cc)				\
	__GEN_RMWcc(op " " arg0, var, cc)

#define GEN_BINARY_RMWcc(op, var, vcon, val, arg0, cc)			\
	__GEN_RMWcc(op " %2, " arg0, var, cc, vcon (val))

#endif /* CC_HAVE_ASM_GOTO */

#endif /* _ASM_X86_RMWcc */
