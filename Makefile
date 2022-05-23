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

# 'dir' is used in here to increase readibity, the current usage has no other effect.

NAME		=	ft_irc
SRCS		=	srcs/main.cpp srcs/Server.cpp srcs/Channel.cpp srcs/User.cpp
CXX			=	g++ 
CXXFLAGS	=	-Wall -Wextra -Werror -std=c++98 #-g3 -fsanitize=address
RM			=	rm -f
OBJS		=	$(SRCS:.cpp=.o)

LIBFT_DIR = libft/
LIBFT_LINK = -L $(dir $(LIBFT_DIR)) -lft
LIBFT = libft.a

INC_DIR = includes/

UNAME := $(shell uname)
# Compiles using threads 
# See https://stackoverflow.com/questions/4778389/automatically-setting-jobs-j-flag-for-a-multicore-machine (Linux)
# See https://stackoverflow.com/questions/1715580/how-to-discover-number-of-logical-cores-on-mac-os-x (MacosX)
ifeq ($(UNAME), Linux)
export MAKEFLAGS="-j $(nproc --all)"
endif
ifeq ($(UNAME), Darwin)
export MAKEFLAGS="-j $(sysctl -n hw.ncpu)"
endif

all: 		$(NAME)

%.o: 		%.cpp
			$(CXX) $(CXXFLAGS) -I $(dir $(LIBFT_DIR)) -I $(dir $(INC_DIR)) -c $< -o $@

$(LIBFT_DIR)$(LIBFT): \
			$(LIBFT_DIR)
			make -C $(dir $(LIBFT_DIR))

# See https://stackoverflow.com/questions/42586080/gcc-linking-object-files-with-warning-optimization-flags
$(NAME): 	$(OBJS) $(dir $(LIBFT_DIR))$(LIBFT)
			$(CXX) $(OBJS) $(CXXFLAGS) $(LIBFT_LINK) -o  $@

clean:
			make -C $(dir $(LIBFT_DIR)) clean
			$(RM) $(OBJS)

fclean:		clean
			make -C $(dir $(LIBFT_DIR)) fclean
			$(RM) $(NAME)

re: 		clean all

.PHONY:		all clean fclean re
