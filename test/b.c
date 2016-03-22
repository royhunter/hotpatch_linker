
extern int x_global;

int y_global;
int z_global;

int init_global = 23;


void swap_b( int *a, int *b)
{
    static int c;
    static int d;
    char *var_str = "aaaaa";
	*a = x_global;
	*b = y_global;
    *a = z_global;
    c = 2;
    d = 3;


}