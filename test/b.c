
extern int x_global;

void swap( int *a, int *b)
{
	*a = x_global;
	*b = x_global;
}