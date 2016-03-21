
extern int x_global;

int y_global;
int z_global;

void swap_b( int *a, int *b)
{
    static int c;
    static int d;
	*a = x_global;
	*b = y_global;
    *a = z_global;
    c = 2;
    d = 3;
}