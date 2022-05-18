# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: pmedina- <3 carce-bo	                    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2022/03/23 22:21:34 by pmedina-          #+#    #+#              #
#    Updated: 2022/04/21 21:32:41 by pmedina-         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME	=	ft_irc
SSRCS	=	srcs/main.cpp srcs/Server.cpp srcs/Channel.cpp srcs/User.cpp
CXX		=	g++ 
CXXFLAGS	=	-Wall -Wextra -Werror -std=c++98 #-g3 -fsanitize=address
RM		=	rm -f
OBJSS		=	$(SSRCS:.cpp=.o)

LIBFT_DIR = libft/
LIBFT_MAC = -L $(LIBFT_DIR) -lft
LIBFT = libft.a

all: $(NAME)

%.o: %.cpp %.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(LIBFT_DIR)$(LIBFT): $(LIBFT_DIR)
	make -C $(dir $(LIBFT_DIR))

$(NAME): $(OBJSS) $(LIBFT_DIR)$(LIBFT)
	@$(CXX) $(OBJSS) $(LIBFT_MAC) -o  $@
	make -C $(dir $(LIBFT_DIR)) clean

clean:
	@$(RM) $(OBJSS) $(OBJSC)
	
fclean	:	clean
			@$(RM) $(SNAME)
			make -C $(dir $(LIBFT_DIR)) fclean
			$(RM) $(NAME)

re	: clean all
