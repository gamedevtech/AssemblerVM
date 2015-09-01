
#ifndef _INCLUDED_VM_H
#define _INCLUDED_VM_H


typedef struct vmunit_t
{
	unsigned char	*machine_code;					/* Codes for the VM to interpret */
	unsigned char	*IP;							/* Instruction pointer */
	unsigned long	code_size;						/* Size in bytes */

	unsigned char	*heap;							/* Local heap for variable allocation */
	unsigned long	heap_size;						/* Size in bytes */

	signed long		*stack;							/* Unit's stack */
	signed long		*SP;							/* Stack pointer */
	unsigned long	stack_size;						/* Size in dwords */

	void (**call_function)(struct vmunit_t *);	/* Registered function call list */

	unsigned char	active;
} VMUnit;


extern	VMUnit			*VM_Load(unsigned char *filename);
extern	void			VM_Close(VMUnit *vm_unit);
extern	unsigned long	VM_StepInstruction(VMUnit *vm_unit);
extern	void			VM_Execute(VMUnit *vm_unit);
extern	unsigned char	VM_RegisterFunction(VMUnit *vm_unit, void (*function)(VMUnit *), unsigned char number);


void __inline StackPush(VMUnit *vm_unit, signed long value)
{
	vm_unit->SP--;
	*vm_unit->SP = value;
}


signed long __inline StackPop(VMUnit *vm_unit)
{
	signed long	value;

	value = *vm_unit->SP;
	vm_unit->SP++;

	return (value);
}


#endif