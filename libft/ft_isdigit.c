#include "libft.h"

int	ft_isdigit(int c)
{
	int	boolean;

	boolean = 1;
	if (c < '0' || c > '9')
		boolean = 0;
	return (boolean);
}
