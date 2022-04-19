# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: pmedina- <3 carce-bo	                    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2022/03/23 22:21:34 by pmedina-          #+#    #+#              #
#    Updated: 2022/04/19 21:59:21 by pmedina-         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

SNAME	=	server
SSRCS	=	srcs/main.cpp srcs/Server.cpp srcs/Channel.cpp srcs/User.cpp
CNAME	=	client
CSRC	=	srcs/example_client.cpp
CXX		=	g++ 
CXXFLAGS	=	-Wall -Wextra -Werror -std=c++98 #-g3 -fsanitize=address
RM		=	rm -f
OBJSS		=	$(SSRCS:.cpp=.o)
OBJSC		=	$(CSRC:.cpp=.o)

%.o: %.cpp %.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

NAME: $(OBJSS) $(OBJSC)
	@$(CXX) $(OBJSS) -o $(SNAME)
	@$(CXX) $(OBJSC) -o $(CNAME)

clean:
	@$(RM) $(OBJSS) $(OBJSC)
	
fclean		:	clean
				@$(RM) $(SNAME) $(CNAME)

re			: clean all
