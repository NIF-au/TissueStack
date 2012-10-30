DIRSRC		=	./

SRCS		=	$(DIRSRC)utils.c 

INCLUDE		=	-I./

LIB_PATH	=	

NAME		=	utils

FLAGS		=	-g3 -O3 -Wall -Werror

all: compile

compile:	
	@echo "\n================== Compile $(NAME) ===================="
	gcc $(SRCS) -o $(NAME) $(INCLUDE) $(FLAGS)

clean:
	@echo "\n==================== Cleaning $(NAME) ====================="
	rm -rf *~ \#*\# $(NAME)
