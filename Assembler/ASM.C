#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <malloc.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>


#define	OPCODE_PUSH_ADDRESS				0x01
#define	OPCODE_PUSH_VALUE				0x02
#define	OPCODE_POP_ADDRESS				0x03
#define	OPCODE_POP_NONE					0x04
#define	OPCODE_SP_PLUS					0x05
#define	OPCODE_SP_MINUS					0x06
#define	OPCODE_ADD						0x07
#define	OPCODE_SUBTRACT					0x08
#define	OPCODE_MULTIPLY					0x09
#define	OPCODE_DIVIDE					0x0A
#define OPCODE_CALL_EXTERN				0x0B
#define	OPCODE_JMP						0x0C
#define	OPCODE_INC						0x0D
#define	OPCODE_DEC						0x0E
#define	OPCODE_JG						0x0F
#define OPCODE_JL						0x10
#define OPCODE_JE						0x11
#define OPCODE_JGE						0x12
#define OPCODE_JLE						0x13
#define	OPCODE_CALL						0x14
#define	OPCODE_RET						0x15
#define	OPCODE_AND						0x16
#define	OPCODE_OR						0x17
#define	OPCODE_XOR						0x18
#define	OPCODE_NOT						0x19
#define	OPCODE_EXIT						0xFF


typedef unsigned char	uBYTE;
typedef unsigned short	uWORD;
typedef unsigned long	uDWORD;
typedef signed char		sBYTE;
typedef signed short	sWORD;
typedef signed long		sDWORD;

typedef struct hvar_t
{
	uBYTE		name[64];
	sDWORD		address;
	sDWORD		value;

	struct hvar_t	*next;
} HeapVariable;

typedef struct extf_t
{
	uBYTE		name[64];
	sDWORD		value;

	struct extf_t	*next;
} ExternFunction;

typedef struct label_t
{
	uBYTE		name[64];
	sDWORD		address;

	struct label_t	*next;
} Label;

typedef struct
{
	FILE		*fp;

	uDWORD		*line_length;
	uDWORD		*line_table;

	uBYTE		current_line[1024];
	uBYTE		current_argument[256];
	uBYTE		current_command[256];

	uDWORD		curline_length;
	uDWORD		curline_number;
	uDWORD		nb_lines;
	uDWORD		filelength;

	HeapVariable	*first_var;
	HeapVariable	*last_var;
	uDWORD			current_address;
	uDWORD			code_length;
	uDWORD			stack_size;

	Label			*first_label;
	Label			*last_label;

	ExternFunction	*first_func;
	ExternFunction	*last_func;
} ASMFile;

typedef struct
{
	uBYTE		*name;
	void		(*handler)(ASMFile *asm_file, FILE *fp);
	uDWORD		size;
	uDWORD		opcode;
} ASMCommand;


uBYTE ReadFileInformation(ASMFile *asm_file)
{
	uDWORD	x, y = 0, opos;
	sDWORD	l = 0;
	uBYTE	inchar;

	if ((asm_file->line_length = malloc(65536 * sizeof(uDWORD))) == NULL)
		return (0);

	if ((asm_file->line_table = malloc(65536 * sizeof(uDWORD))) == NULL)
	{
		free(asm_file->line_length);
		return (0);
	}

	fseek(asm_file->fp, 0, SEEK_END);
	asm_file->filelength = ftell(asm_file->fp);
	fseek(asm_file->fp, 0, SEEK_SET);
	asm_file->line_table[y] = 0;

	for (x = 0; x < asm_file->filelength; x++)
	{
		l++;
		inchar = fgetc(asm_file->fp);
		opos = ftell(asm_file->fp);

		if (inchar == 13)
		{
			inchar = fgetc(asm_file->fp);
			if (inchar == 10)
			{
				if (y != 0) asm_file->line_length[y] = l - 2;
				else asm_file->line_length[y] = l - 1;

				y++;
				asm_file->line_table[y] = ftell(asm_file->fp);
				l = 0;
			}
		}

		fseek(asm_file->fp, opos, SEEK_SET);
	}

	asm_file->nb_lines = y;

	if (y == 0)
	{
		free(asm_file->line_length);
		free(asm_file->line_table);
		return (0);
	}

	return (1);
}


ASMFile *OpenASMFile(uBYTE *filename)
{
	ASMFile		*asm_file;
	FILE		*fp;

	if ((fp = fopen(filename, "rb")) == NULL)
		return (NULL);

	if ((asm_file = malloc(sizeof(ASMFile))) == NULL)
	{
		fclose(fp);
		return (NULL);
	}

	asm_file->fp = fp;
	asm_file->first_var = NULL;
	asm_file->last_var = NULL;
	asm_file->first_label = NULL;
	asm_file->last_label = NULL;
	asm_file->current_address = 0;
	asm_file->code_length = 0;
	asm_file->stack_size = 256;
	asm_file->first_func = 0;
	asm_file->last_func = 0;

	if (!ReadFileInformation(asm_file))
	{
		free(asm_file);
		fclose(fp);
		return (NULL);
	}

	return (asm_file);
}


void CloseASMFile(ASMFile *asm_file)
{
	HeapVariable	*current_var, *next_var;
	Label			*current_label, *next_label;
	ExternFunction	*current_func, *next_func;

	if (asm_file)
	{
		fclose(asm_file->fp);
		free(asm_file->line_table);
		free(asm_file->line_length);

		for (current_var = asm_file->first_var; current_var; current_var = next_var)
		{
			next_var = current_var->next;
			free(current_var);
		}

		for (current_label = asm_file->first_label; current_label; current_label = next_label)
		{
			next_label = current_label->next;
			free(current_label);
		}

		for (current_func = asm_file->first_func; current_func; current_func = next_func)
		{
			next_func = current_func->next;
			free(current_func);
		}

		free(asm_file);
	}
}


void ExitError(ASMFile *asm_file)
{
	CloseASMFile(asm_file);
	exit(0);
}


void ReadLine(ASMFile *asm_file, uDWORD line_num)
{
	uDWORD	x;

	if (line_num > asm_file->nb_lines) return;
	fseek(asm_file->fp, asm_file->line_table[line_num], SEEK_SET);

	for (x = 0; x < asm_file->line_length[line_num]; x++)
		asm_file->current_line[x] = fgetc(asm_file->fp);
	asm_file->current_line[x] = 0;

	asm_file->curline_length = asm_file->line_length[line_num];
	asm_file->curline_number = line_num + 1;
}


void ClearLineSpaces(ASMFile *asm_file)
{
	uWORD	x = 0, y;
	uBYTE	in_address = 0;

	// Get rid of any spaces that exist before the line starts

	while (x < asm_file->curline_length)
	{
		if (asm_file->current_line[x] != 32) break;

		if (asm_file->current_line[x] == 32)
		{
			for (y = x; y < asm_file->curline_length; y++)
				asm_file->current_line[y] = asm_file->current_line[y + 1];

			asm_file->curline_length--;
		}
		else x++;
	}

	// Reduce the spaces between arguments to 1 space

	x = 0;
	while (x < asm_file->curline_length)
	{
		// Handle the reduction of all spaces within an address bracket

		if (asm_file->current_line[x] == '[')
		{
			in_address = 1;
			x++;

			continue;
		}

		if (in_address && asm_file->current_line[x] == 32)
		{
			for (y = x; y < asm_file->curline_length; y++)
				asm_file->current_line[y] = asm_file->current_line[y + 1];

			asm_file->curline_length--;

			continue;
		}

		if (in_address && asm_file->current_line[x] == ']')
		{
			in_address = 0;
			x++;
			
			continue;
		}

		// Handle normal spaces

		if (asm_file->current_line[x] == 32 && asm_file->current_line[x + 1] == 32)
		{
			for (y = x; y < asm_file->curline_length; y++)
				asm_file->current_line[y] = asm_file->current_line[y + 1];

			asm_file->curline_length--;

			continue;
		}

		x++;
	}
}


void ReadCommand(ASMFile *asm_file)
{
	uWORD	x = 0;

	while (x < asm_file->curline_length)
	{
		if (asm_file->current_line[x] == 32) break;
		asm_file->current_command[x] = asm_file->current_line[x];

		if (asm_file->current_command[x] > 96 && asm_file->current_command[x] < 123)
			asm_file->current_command[x] -= 0x20;

		x++;
	}

	asm_file->current_command[x] = 0;
}


void ReadArgument(ASMFile *asm_file, uBYTE argnum)
{
	uWORD	x = 0, y, z, ns = 0;

	for (y = 0; y < argnum + 1; y++)
	{
		z = 0;

		while (x < asm_file->curline_length)
		{
			if (asm_file->current_line[x] == 32)
			{
				x++;
				break;
			}

			asm_file->current_argument[z] = asm_file->current_line[x];

			if (asm_file->current_argument[z] > 96 && asm_file->current_argument[z] < 123)
				asm_file->current_argument[z] -= 0x20;

			x++; z++;
		}
	}

	asm_file->current_argument[z] = 0;
}


void TestASMFile(ASMFile *asm_file)
{
	uDWORD	x;

	printf("\n");
	printf("================================================\n");
	printf("Testing loaded ASM file\n");
	printf("================================================\n\n");

	for (x = 0; x < asm_file->nb_lines; x++)
	{
		ReadLine(asm_file, x);
		ClearLineSpaces(asm_file);
		ReadCommand(asm_file);

		printf("%20s : %s\n", asm_file->current_command, asm_file->current_line);
	}

	printf("\n");
	printf("= End of Test ==================================\n");
}


//===========================================================================================
//--------------------------------------------------------------------------> Assembler Code
//===========================================================================================


sDWORD AddNewVariable(ASMFile *asm_file, uBYTE *name, sDWORD value)
{
	HeapVariable	*new_var;

	new_var = malloc(sizeof(HeapVariable));

	if (asm_file->first_var == NULL)
	{
		new_var->next = NULL;
		asm_file->first_var = new_var;
		asm_file->last_var = new_var;
	}
	else
	{
		new_var->next = NULL;
		asm_file->last_var->next = new_var;
		asm_file->last_var = new_var;
	}

	ReadArgument(asm_file, 2);

	strcpy(new_var->name, name);
	new_var->address = asm_file->current_address;
	new_var->value = value;

	asm_file->current_address += 4;

	return (asm_file->current_address - 4);
}


void HandleCommand_NewVariable(ASMFile *asm_file, FILE *fp)
{
	if (asm_file->current_command[strlen(asm_file->current_command) - 1] == ':')
	{
		Label	*new_label;

		new_label = malloc(sizeof(Label));

		if (asm_file->first_label == NULL)
		{
			new_label->next = NULL;
			asm_file->first_label = new_label;
			asm_file->last_label = new_label;
		}
		else
		{
			new_label->next = NULL;
			asm_file->last_label->next = new_label;
			asm_file->last_label = new_label;
		}

		asm_file->current_command[strlen(asm_file->current_command) - 1] = 0;
		strcpy(new_label->name, asm_file->current_command);
		new_label->address = asm_file->code_length;
	}
	else
	{
		ReadArgument(asm_file, 1);

		if (!strcmp(asm_file->current_argument, "V"))
		{
			HeapVariable	*new_var;

			new_var = malloc(sizeof(HeapVariable));

			if (asm_file->first_var == NULL)
			{
				new_var->next = NULL;
				asm_file->first_var = new_var;
				asm_file->last_var = new_var;
			}
			else
			{
				new_var->next = NULL;
				asm_file->last_var->next = new_var;
				asm_file->last_var = new_var;
			}

			ReadArgument(asm_file, 2);

			strcpy(new_var->name, asm_file->current_command);
			new_var->address = asm_file->current_address;
			sscanf(asm_file->current_argument, "%d",&new_var->value);

			asm_file->current_address += 4;
		}

		else if (!strcmp(asm_file->current_argument, "F"))
		{
			ExternFunction	*new_func;

			new_func = malloc(sizeof(ExternFunction));

			if (asm_file->first_func == NULL)
			{
				new_func->next = NULL;
				asm_file->first_func = new_func;
				asm_file->last_func = new_func;
			}
			else
			{
				new_func->next = NULL;
				asm_file->last_func->next = new_func;
				asm_file->last_func = new_func;
			}

			ReadArgument(asm_file, 2);

			strcpy(new_func->name, asm_file->current_command);
			sscanf(asm_file->current_argument, "%d", &new_func->value);
		}
	}
}


void ClearVariableBrackets(uBYTE *string)
{
	uDWORD	x;

	for (x = 0; x < strlen(string); x++)
		string[x] = string[x + 1];

	string[strlen(string) - 1] = 0;
}


uBYTE VerifyNumber(uBYTE *string)
{
	uDWORD	x;

	for (x = 0; string[x]; x++)
		if (string[x] < '0' || string[x] > '9')
			return (0);

	return (1);
}


sDWORD GetStringAddress(ASMFile *asm_file, uBYTE *varstring)
{
	uDWORD	x = 0, y = 0;
	sDWORD	address = 0, value, prev_op = 1;
	uBYTE	current_var[64] = { 0 };
	uBYTE	op_assigned = 1, last_pass = 0;;

	while (varstring[x])
	{
		// Read the current non-operation token
		if (varstring[x] != '+' && varstring[x] != '-')
		{
			current_var[y++] = varstring[x++];
			continue;
		}

address_modify:;
		
		// Complete the string
		current_var[y] = 0;
		y = 0;

		if (current_var[0])
		{
			// If the string contains just a number, scan it into the value
			if (VerifyNumber(current_var))
			{
				sscanf(current_var, "%d", &value);
				value <<= 2;
			}

			// Else locate the address of the variable the string represents
			else
			{
				HeapVariable	*cur_var;

				value = 0;
				for (cur_var = asm_file->first_var; cur_var; cur_var = cur_var->next)
					if (!strcmp(current_var, cur_var->name))
					{
						value = cur_var->address;
						break;
					}

				if (cur_var == NULL)
				{
					printf("ERROR (%d): Illegal variable name, %s\n", asm_file->curline_number, current_var);
					ExitError(asm_file);
				}
			}

			// Modify the current address
			address += (prev_op * value);
			op_assigned = 0;

			if (last_pass) return (address);
		}

		// If an operator for the next modification has already been assigned, modify the
		// current operation
		if (op_assigned)
		{
			if (varstring[x] == '+') prev_op *= 1;
			if (varstring[x] == '-') prev_op *= -1;
		}
		else
		{
			if (varstring[x] == '+') prev_op = 1;
			if (varstring[x] == '-') prev_op = -1;
			op_assigned = 1;
		}

		x++;
	}

	// Finally, process the last read argument
	last_pass = 1;
	goto address_modify;
}


void WriteVariableAddress(ASMFile *asm_file, uBYTE *varname, FILE *fp)
{
	sDWORD	address;

	address = GetStringAddress(asm_file, varname);
	fwrite(&address, 1, sizeof(sDWORD), fp);
}


void WriteLabelAddress(ASMFile *asm_file, uBYTE *label_name, FILE *fp)
{
	Label	*cur_label;

	for (cur_label = asm_file->first_label; cur_label; cur_label = cur_label->next)
		if (!strcmp(label_name, cur_label->name))
		{
			fwrite(&cur_label->address, 1, sizeof(sDWORD), fp);
			break;
		}

	if (cur_label == NULL)
	{
		printf("ERROR (%d): Illegal label name, %s\n", asm_file->curline_number, label_name);
		ExitError(asm_file);
	}
}


void WriteFunctionNumber(ASMFile *asm_file, uBYTE *func_name, FILE *fp)
{
	ExternFunction	*cur_func;

	for (cur_func = asm_file->first_func; cur_func; cur_func = cur_func->next)
		if (!strcmp(func_name, cur_func->name))
		{
			fwrite(&cur_func->value, 1, sizeof(sDWORD), fp);
			break;
		}

	if (cur_func == NULL)
	{
		printf("ERROR (%d): Illegal external C function name, %s\n", asm_file->curline_number, func_name);
		ExitError(asm_file);
	}
}


uBYTE IsArgumentAddress(uBYTE *argument)
{
	uDWORD	s;

	s = strlen(argument);

	if (argument[0] == '[' && argument[s - 1] == ']')
		return (1);

	return (0);
}


void HandleCommand_PUSH(ASMFile *asm_file, FILE *fp)
{
	ReadArgument(asm_file, 1);
		
	if (IsArgumentAddress(asm_file->current_argument))
	{
		fputc(OPCODE_PUSH_ADDRESS, fp);
		ClearVariableBrackets(asm_file->current_argument);
		WriteVariableAddress(asm_file, asm_file->current_argument, fp);
	}
	else
	{
		fputc(OPCODE_PUSH_VALUE, fp);

		if (VerifyNumber(asm_file->current_argument))
		{
			sDWORD	value;

			sscanf(asm_file->current_argument, "%d", &value);
			fwrite(&value, 1, sizeof(sDWORD), fp);
		}
		else
		{
			WriteVariableAddress(asm_file, asm_file->current_argument, fp);
		}
	}
}


void HandleCommand_POP(ASMFile *asm_file, FILE *fp)
{
	ReadArgument(asm_file, 1);

	fputc(OPCODE_POP_ADDRESS, fp);
	ClearVariableBrackets(asm_file->current_argument);
	WriteVariableAddress(asm_file, asm_file->current_argument, fp);
}


void HandleCommand_SPP(ASMFile *asm_file, FILE *fp)
{
	sDWORD	value;

	ReadArgument(asm_file, 1);

	fputc(OPCODE_SP_PLUS, fp);
	sscanf(asm_file->current_argument, "%d", &value);
	fwrite(&value, 1, sizeof(sDWORD), fp);
}


void HandleCommand_SPM(ASMFile *asm_file, FILE *fp)
{
	sDWORD	value;

	ReadArgument(asm_file, 1);

	fputc(OPCODE_SP_MINUS, fp);
	sscanf(asm_file->current_argument, "%d", &value);
	fwrite(&value, 1, sizeof(sDWORD), fp);
}


void HandleCommand_CALLC(ASMFile *asm_file, FILE *fp)
{
	ReadArgument(asm_file, 1);

	fputc(OPCODE_CALL_EXTERN, fp);
	WriteFunctionNumber(asm_file, asm_file->current_argument, fp);
}


void HandleCommand_JMP(ASMFile *asm_file, FILE *fp)
{
	ReadArgument(asm_file, 1);

	fputc(OPCODE_JMP, fp);
	WriteLabelAddress(asm_file, asm_file->current_argument, fp);
}


void HandleCommand_JG(ASMFile *asm_file, FILE *fp)
{
	ReadArgument(asm_file, 1);

	fputc(OPCODE_JG, fp);
	WriteLabelAddress(asm_file, asm_file->current_argument, fp);
}


void HandleCommand_JL(ASMFile *asm_file, FILE *fp)
{
	ReadArgument(asm_file, 1);

	fputc(OPCODE_JL, fp);
	WriteLabelAddress(asm_file, asm_file->current_argument, fp);
}


void HandleCommand_JE(ASMFile *asm_file, FILE *fp)
{
	ReadArgument(asm_file, 1);

	fputc(OPCODE_JE, fp);
	WriteLabelAddress(asm_file, asm_file->current_argument, fp);
}


void HandleCommand_JGE(ASMFile *asm_file, FILE *fp)
{
	ReadArgument(asm_file, 1);

	fputc(OPCODE_JGE, fp);
	WriteLabelAddress(asm_file, asm_file->current_argument, fp);
}


void HandleCommand_JLE(ASMFile *asm_file, FILE *fp)
{
	ReadArgument(asm_file, 1);

	fputc(OPCODE_JLE, fp);
	WriteLabelAddress(asm_file, asm_file->current_argument, fp);
}


void HandleCommand_CALL(ASMFile *asm_file, FILE *fp)
{
	ReadArgument(asm_file, 1);

	fputc(OPCODE_CALL, fp);
	WriteLabelAddress(asm_file, asm_file->current_argument, fp);
}


ASMCommand	CommandList[] =
{
	{ "PUSH",	HandleCommand_PUSH,			5,	0					},
	{ "POP",	HandleCommand_POP,			5,	0					},
	{ "POPN",	NULL,						1,	OPCODE_POP_NONE		},
	{ "SP+",	HandleCommand_SPP,			5,	0					},
	{ "SP-",	HandleCommand_SPM,			5,	0					},
	{ "ADD",	NULL,						1,	OPCODE_ADD			},
	{ "SUB",	NULL,						1,	OPCODE_SUBTRACT		},
	{ "MUL",	NULL,						1,	OPCODE_MULTIPLY		},
	{ "DIV",	NULL,						1,	OPCODE_DIVIDE		},
	{ "CALLC",	HandleCommand_CALLC,		5,	0					},
	{ "JMP",	HandleCommand_JMP,			5,	0					},
	{ "INC",	NULL,						1,	OPCODE_INC			},
	{ "DEC",	NULL,						1,	OPCODE_DEC			},
	{ "JG",		HandleCommand_JG,			5,	0					},
	{ "JL",		HandleCommand_JL,			5,	0					},
	{ "JE",		HandleCommand_JE,			5,	0					},
	{ "JGE",	HandleCommand_JGE,			5,	0					},
	{ "JLE",	HandleCommand_JLE,			5,	0					},
	{ "CALL",	HandleCommand_CALL,			5,	0					},
	{ "RET",	NULL,						1,	OPCODE_RET			},
	{ "AND",	NULL,						1,	OPCODE_AND			},
	{ "OR",		NULL,						1,	OPCODE_OR			},
	{ "XOR",	NULL,						1,	OPCODE_XOR			},
	{ "NOT",	NULL,						1,	OPCODE_NOT			},
	{ "EXIT",	NULL,						1,	OPCODE_EXIT			}
};


void FormatFilename(uBYTE *filename)
{
	uDWORD	s = strlen(filename);

	filename[s - 1] = 'U';
	filename[s - 2] = 'M';
	filename[s - 3] = 'V';
}


uBYTE CompileASMFile(ASMFile *asm_file, uBYTE *filename)
{
	FILE			*fp;
	uDWORD			signature = 'MVTQ';
	uDWORD			x, y;
	HeapVariable	*current_var;

	FormatFilename(filename);

	if ((fp = fopen(filename, "wb")) == NULL)
		return (0);

	fwrite(&signature, 1, sizeof(uDWORD), fp);
	fprintf(fp, "xxxxxxxxxxxx");

	// Process all variables and labels first

	AddNewVariable(asm_file, "SP", 0);
	AddNewVariable(asm_file, "BP", 0);

	for (x = 0; x < asm_file->nb_lines; x++)
	{
		ReadLine(asm_file, x);
		ClearLineSpaces(asm_file);
		ReadCommand(asm_file);

		if (asm_file->current_command[0] == 0)
			continue;

		if (asm_file->current_command[0] == ';')
			continue;

		for (y = 0; y < sizeof(CommandList) / sizeof(CommandList[0]); y++)
			if (!strcmp(asm_file->current_command, CommandList[y].name))
			{
				asm_file->code_length += CommandList[y].size;
				break;
			}

		if (y == sizeof(CommandList) / sizeof(CommandList[0]))
			HandleCommand_NewVariable(asm_file, fp);
	}

	asm_file->code_length = 0;

	// Now generate the code

	for (x = 0; x < asm_file->nb_lines; x++)
	{
		ReadLine(asm_file, x);
		ClearLineSpaces(asm_file);
		ReadCommand(asm_file);

		if (asm_file->current_command[0] == 0)
			continue;

		if (asm_file->current_command[0] == ';')
			continue;

		for (y = 0; y < sizeof(CommandList) / sizeof(CommandList[0]); y++)
		{
			if (!strcmp(asm_file->current_command, CommandList[y].name))
			{
				if (CommandList[y].handler)
					CommandList[y].handler(asm_file, fp);
				else
					fputc(CommandList[y].opcode, fp);

				asm_file->code_length += CommandList[y].size;

				break;
			}
		}
	}

	for (current_var = asm_file->first_var; current_var; current_var = current_var->next)
		fwrite(&current_var->value, 1, sizeof(sDWORD), fp);

	fseek(fp, 4, SEEK_SET);
	fwrite(&asm_file->code_length, 1, sizeof(sDWORD), fp);
	fwrite(&asm_file->current_address, 1, sizeof(sDWORD), fp);
	fwrite(&asm_file->stack_size, 1, sizeof(sDWORD), fp);

	fclose(fp);

	return (1);
}


void main(int argc, char *argv[])
{
	ASMFile	*asm_file;

	if (argc != 2)
		return;

	if ((asm_file = OpenASMFile(argv[1])) == NULL)
		return;

	CompileASMFile(asm_file, argv[1]);

	CloseASMFile(asm_file);
}
