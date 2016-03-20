#include <stdio.h>
int x_global;
static int x_static;


void swap( int *a, int *b)
{
	int temp;
	temp = *a;
	*a = *b;
	*b = temp;
}

void main()
{
	int a = 100;
	int b = 88;
	x_global = a;
	x_static = b;
	printf("before x_global is %d, x_static is %d\n", x_global, x_static);
	
	swap( &x_global, &x_static);
	
	printf("after x_global is %d, x_static is %d\n", x_global, x_static);
}