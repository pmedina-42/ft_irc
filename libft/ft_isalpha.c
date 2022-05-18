#include "libft.h"

int	ft_isalpha(int c)
{
	int	boolean;

	boolean = 1;
	if (c < 'A' || (c > 'Z' && c < 'a') || c > 'z')
		boolean = 0;
	return (boolean);
}
