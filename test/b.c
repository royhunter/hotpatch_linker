
extern int x_global;

int y_global;
int z_global;

void swap_b( int *a, int *b)
{
	*a = x_global;
	*b = y_global;
    *a = z_global;
}