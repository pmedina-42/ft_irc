#include "libft.h"

int	ft_isalnum(int c)
{
	int	boolean;

	boolean = 1;
	if (ft_isdigit(c) == 0 && ft_isalpha(c) == 0)
		boolean = 0;
	return (boolean);
}
