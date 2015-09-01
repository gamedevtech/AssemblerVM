
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <math.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include "opengl.h"
#include "main.h"
#include "vm.h"


typedef struct
{
	float	x;
	float	y;
	float	z;
} Point3D;

typedef struct
{
	Point3D	position;
	float	rotation;
} Object;


Object		MainObject = { { 0, 0, 0 }, 0 };
VMUnit		*MainVM;


void ClearScreen(VMUnit *vm_unit)
{
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
}


void DrawObject(VMUnit *vm_unit)
{
	glPushMatrix();
	glTranslatef(MainObject.position.x, MainObject.position.y, MainObject.position.z);
	glRotatef(MainObject.rotation, 0, 0, 1);

	glBegin(GL_TRIANGLES);
	glColor3f(1, 0, 0); glVertex2f(0.0f, 0.1f);
	glColor3f(0, 1, 0); glVertex2f(0.2f, -0.1f);
	glColor3f(0, 0, 1); glVertex2f(-0.2f, -0.1f);
	glEnd();

	glPopMatrix();
}


void SwapScreen(VMUnit *vm_unit)
{
	SwapBuffers(hdc);
}


void SetObjectRotation(VMUnit *vm_unit)
{
	int	rotation;

	rotation = StackPop(vm_unit);

	MainObject.rotation = (float)rotation * (1.0f / 65536.0f);
}


void SetObjectPosition(VMUnit *vm_unit)
{
	int	x, y;

	x = StackPop(vm_unit);
	y = StackPop(vm_unit);

	MainObject.position.x = (float)x * (1.0f / 65536.0f);
	MainObject.position.y = (float)y * (1.0f / 65536.0f);
}


void Sine(VMUnit *vm_unit)
{
	int		angle;
	float	new_angle;
	float	sin_val;

	angle = StackPop(vm_unit);

	// Convert to degress
	new_angle = (float)angle * (1.0f / 65536.0f);

	// Convert to radians
	new_angle = (3.141f / 180.0f) * new_angle;

	sin_val = (float)sin(new_angle);

	sin_val *= 65536.0f;

	StackPush(vm_unit, (int)sin_val);
}


void Cosine(VMUnit *vm_unit)
{
	int		angle;
	float	new_angle;
	float	cos_val;

	angle = StackPop(vm_unit);

	// Convert to degress
	new_angle = (float)angle * (1.0f / 65536.0f);

	// Convert to radians
	new_angle = (3.141f / 180.0f) * new_angle;

	cos_val = (float)cos(new_angle);

	cos_val *= 65536.0f;

	StackPush(vm_unit, (int)cos_val);
}


void Open3DView(VMUnit *vm_unit)
{
	EnableOpenGL(hWndGlobal, &hdc, &hrc);
}


void GetKeyboardCode(VMUnit *vm_unit)
{
	int	keyboard_code = 0;

	if (keytab[VK_UP]) keyboard_code |= 1;
	if (keytab[VK_DOWN]) keyboard_code |= 2;
	if (keytab[VK_LEFT]) keyboard_code |= 4;
	if (keytab[VK_RIGHT]) keyboard_code |= 8;
	if (keytab[VK_ESCAPE]) keyboard_code |= 16;

	StackPush(vm_unit, keyboard_code);
}


void StartFunction(void)
{
	MainVM = VM_Load("test.vmu");

	VM_RegisterFunction(MainVM, ClearScreen, 1);
	VM_RegisterFunction(MainVM, DrawObject, 2);
	VM_RegisterFunction(MainVM, SwapScreen, 3);
	VM_RegisterFunction(MainVM, SetObjectRotation, 4);
	VM_RegisterFunction(MainVM, SetObjectPosition, 5);
	VM_RegisterFunction(MainVM, Sine, 6);
	VM_RegisterFunction(MainVM, Cosine, 7);
	VM_RegisterFunction(MainVM, Open3DView, 8);
	VM_RegisterFunction(MainVM, GetKeyboardCode, 9);
}


void MainLoop(void)
{
	VM_StepInstruction(MainVM);
}


void EndFunction(void)
{
	VM_Close(MainVM);
	DisableOpenGL(hWndGlobal, hdc, hrc);
}