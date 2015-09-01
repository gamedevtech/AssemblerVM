
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <malloc.h>


//===========================================================================================
//-----------------------------------------------------------------------------> Definitions
//===========================================================================================


#define OPCODE_PUSH_AT_ADDRESS		0x01
#define OPCODE_PUSH_VALUE			0x02
#define OPCODE_POP_TO_ADDRESS		0x03
#define OPCODE_POP_NONE				0x04
#define OPCODE_SP_PLUS				0x05
#define OPCODE_SP_MINUS				0x06
#define OPCODE_ADD					0x07
#define OPCODE_SUBTRACT				0x08
#define OPCODE_MULTIPLY				0x09
#define OPCODE_DIVIDE				0x0A
#define OPCODE_CALL_EXTERN			0x0B
#define	OPCODE_JMP					0x0C
#define	OPCODE_INC					0x0D
#define	OPCODE_DEC					0x0E
#define	OPCODE_JG					0x0F
#define OPCODE_JL					0x10
#define OPCODE_JE					0x11
#define OPCODE_JGE					0x12
#define OPCODE_JLE					0x13
#define	OPCODE_CALL					0x14
#define	OPCODE_RET					0x15
#define	OPCODE_AND					0x16
#define	OPCODE_OR					0x17
#define	OPCODE_XOR					0x18
#define	OPCODE_NOT					0x19
#define OPCODE_EXIT					0xFF


//===========================================================================================
//------------------------------------------------------------------------------------> Types
//===========================================================================================


typedef unsigned char	uBYTE;
typedef unsigned short	uWORD;
typedef unsigned long	uDWORD;
typedef signed char		sBYTE;
typedef signed short	sWORD;
typedef signed long		sDWORD;

typedef struct vmunit_t
{
	uBYTE		*machine_code;					/* Codes for the VM to interpret */
	uBYTE		*IP;							/* Instruction pointer */
	uDWORD		code_size;						/* Size in bytes */

	uBYTE		*heap;							/* Local heap for variable allocation */
	uDWORD		heap_size;						/* Size in bytes */

	sDWORD		*stack;							/* Unit's stack */
	sDWORD		*SP;							/* Stack pointer */
	uDWORD		stack_size;						/* Size in dwords */

	void (**call_function)(struct vmunit_t *);	/* Registered function call list */

	uBYTE		active;
} VMUnit;


void __inline StackPush(VMUnit *vm_unit, sDWORD value)
{
	vm_unit->SP--;
	*vm_unit->SP = value;
}


sDWORD __inline StackPop(VMUnit *vm_unit)
{
	signed long	value;

	value = *vm_unit->SP;
	vm_unit->SP++;

	return (value);
}


VMUnit *VM_Load(uBYTE *filename)
{
	FILE	*fp;
	uDWORD	signature, filesize;
	VMUnit	*vm_unit;

	// Open the file
	if ((fp = fopen(filename, "rb")) == NULL)
		return (NULL);

	// Verify the unit signature
	fread(&signature, 1, sizeof(uDWORD), fp);
	if (signature != 'MVTQ')
	{
		fclose(fp);
		return (NULL);
	}

	// Allocate structure memory
	if ((vm_unit = malloc(sizeof(VMUnit))) == NULL)
	{
		fclose(fp);
		return (NULL);
	}

	// Allocate space for the code
	fseek(fp, 0, SEEK_END);
	filesize = ftell(fp);
	fseek(fp, 4, SEEK_SET);
	fread(&vm_unit->code_size, 1, sizeof(uDWORD), fp);
	if ((vm_unit->machine_code = malloc(vm_unit->code_size)) == NULL)
	{
		free(vm_unit);
		fclose(fp);
		return (NULL);
	}

	// Allocate space for the local variable heap
	fread(&vm_unit->heap_size, 1, sizeof(uDWORD),fp);
	if ((vm_unit->heap = malloc(vm_unit->heap_size)) == NULL)
	{
		free(vm_unit->machine_code);
		free(vm_unit);
		return (NULL);
	}

	// Allocate space for the stack
	fread(&vm_unit->stack_size, 1, sizeof(uDWORD), fp);
	if ((vm_unit->stack = malloc(vm_unit->stack_size)) == NULL)
	{
		free(vm_unit->machine_code);
		free(vm_unit->heap);
		free(vm_unit);
		return (NULL);
	}
	vm_unit->stack_size >>= 2;

	// Read in all the code and the data
	fread(vm_unit->machine_code, 1, vm_unit->code_size, fp);
	fread(vm_unit->heap, 1, vm_unit->heap_size, fp);

	// Finally setup the unit registers
	vm_unit->IP = vm_unit->machine_code;
	vm_unit->SP = vm_unit->stack + vm_unit->stack_size - 1;
	*(sDWORD *)vm_unit->heap = vm_unit->stack_size - 1;
	vm_unit->active = 1;

	vm_unit->call_function = malloc(200 * sizeof(void (*)(void)));

	fclose(fp);

	return (vm_unit);
}


void VM_Close(VMUnit *vm_unit)
{
	// Just deallocate some memory
	if (vm_unit)
	{
		if (vm_unit->machine_code) free(vm_unit->machine_code);
		if (vm_unit->heap) free(vm_unit->heap);
		if (vm_unit->stack) free(vm_unit->stack);
		if (vm_unit->call_function) free(vm_unit->call_function);
		free(vm_unit);
	}
}


uBYTE VM_GetByte(VMUnit *vm_unit)
{
	// Retrieve a byte from the machine code and increment the instruction pointer
	return (*vm_unit->IP++);
}


sDWORD VM_GetInt(VMUnit *vm_unit)
{
	sDWORD	value;

	// Retrieve an integer from the machine code and add 4 bytes to the instruction pointer
	value = *((sDWORD *)vm_unit->IP);
	vm_unit->IP += sizeof(sDWORD);

	return (value);
}


sDWORD VM_StepInstruction(VMUnit *vm_unit)
{
	uBYTE	opcode;
	sDWORD	value, address;

	if (!vm_unit->active)
		return (0);

	opcode = VM_GetByte(vm_unit);

	switch (opcode)
	{
		case (OPCODE_PUSH_AT_ADDRESS):
			value = VM_GetInt(vm_unit);
			value = *(sDWORD *)(vm_unit->heap + value);
			StackPush(vm_unit, value);
			break;

		case (OPCODE_PUSH_VALUE):
			value = VM_GetInt(vm_unit);
			StackPush(vm_unit, value);
			break;

		case (OPCODE_POP_TO_ADDRESS):
			value = VM_GetInt(vm_unit);
			*((sDWORD *)(vm_unit->heap + value)) = StackPop(vm_unit);
			break;

		case (OPCODE_POP_NONE):
			StackPop(vm_unit);
			break;

		case (OPCODE_SP_PLUS):
			value = VM_GetInt(vm_unit);
			vm_unit->SP += value;
			*(sDWORD *)vm_unit->heap += value;
			break;

		case (OPCODE_SP_MINUS):
			value = VM_GetInt(vm_unit);
			vm_unit->SP -= value;
			*(sDWORD *)vm_unit->heap -= value;
			break;

		case (OPCODE_ADD):
			value = StackPop(vm_unit) + StackPop(vm_unit);
			StackPush(vm_unit, value);
			break;

		case (OPCODE_SUBTRACT):
			value = StackPop(vm_unit);
			value = StackPop(vm_unit) - value;
			StackPush(vm_unit, value);
			break;

		case (OPCODE_MULTIPLY):
			value = StackPop(vm_unit) * StackPop(vm_unit);
			StackPush(vm_unit, value);
			break;

		case (OPCODE_DIVIDE):
			value = StackPop(vm_unit);
			value = StackPop(vm_unit) / value;
			StackPush(vm_unit, value);
			break;

		case (OPCODE_CALL_EXTERN):
			value = VM_GetInt(vm_unit);
			vm_unit->call_function[value](vm_unit);
			break;

		case (OPCODE_JMP):
			value = VM_GetInt(vm_unit);
			vm_unit->IP = vm_unit->machine_code + value;
			break;

		case (OPCODE_INC):
			value = StackPop(vm_unit);
			value++;
			StackPush(vm_unit, value);
			break;

		case (OPCODE_DEC):
			value = StackPop(vm_unit);
			value--;
			StackPush(vm_unit, value);
			break;

		case (OPCODE_JG):
			address = VM_GetInt(vm_unit);
			value = StackPop(vm_unit);
			if (StackPop(vm_unit) > value)
				vm_unit->IP = vm_unit->machine_code + address;
			break;

		case (OPCODE_JL):
			address = VM_GetInt(vm_unit);
			value = StackPop(vm_unit);
			if (StackPop(vm_unit) < value)
				vm_unit->IP = vm_unit->machine_code + address;
			break;

		case (OPCODE_JE):
			address = VM_GetInt(vm_unit);
			value = StackPop(vm_unit);
			if (StackPop(vm_unit) == value)
				vm_unit->IP = vm_unit->machine_code + address;
			break;

		case (OPCODE_JGE):
			address = VM_GetInt(vm_unit);
			value = StackPop(vm_unit);
			if (StackPop(vm_unit) >= value)
				vm_unit->IP = vm_unit->machine_code + address;
			break;

		case (OPCODE_JLE):
			address = VM_GetInt(vm_unit);
			value = StackPop(vm_unit);
			if (StackPop(vm_unit) <= value)
				vm_unit->IP = vm_unit->machine_code + address;
			break;

		case (OPCODE_CALL):
			value = VM_GetInt(vm_unit);
			StackPush(vm_unit, (sDWORD)(vm_unit->IP - vm_unit->machine_code));
			vm_unit->IP = vm_unit->machine_code + value;
			break;

		case (OPCODE_RET):
			vm_unit->IP = vm_unit->machine_code + StackPop(vm_unit);
			break;

		case (OPCODE_AND):
			value = StackPop(vm_unit);
			address = StackPop(vm_unit);
			value &= address;
			StackPush(vm_unit, value);
			break;

		case (OPCODE_OR):
			value = StackPop(vm_unit) | StackPop(vm_unit);
			StackPush(vm_unit, value);
			break;

		case (OPCODE_XOR):
			value = StackPop(vm_unit) ^ StackPop(vm_unit);
			StackPush(vm_unit, value);
			break;

		case (OPCODE_NOT):
			value = ~StackPop(vm_unit);
			StackPush(vm_unit, value);
			break;

		case (OPCODE_EXIT):
			vm_unit->active = 0;
			return (0);
			break;
	}

	return (1);
}


void VM_Execute(VMUnit *vm_unit)
{
	uBYTE	execute = 1;

	while (VM_StepInstruction(vm_unit))
		;
}


uBYTE VM_RegisterFunction(VMUnit *vm_unit, void (*function)(VMUnit *), uBYTE number)
{
	if (function == NULL)
		return (0);

	vm_unit->call_function[number] = function;

	return (1);
}
