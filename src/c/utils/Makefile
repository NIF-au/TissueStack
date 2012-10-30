DIRSRC		=	./

SRCS		=	$(DIRSRC)utils.c 

INCLUDE		=	-I./ -I../headers

LIB_PATH	=	

NAME		=	utils

FLAGS		=	-ggdb -O3 -Wall -Werror -Wno-unused-result

all: compile

compile:	
	@echo "\n================== Compile $(NAME) ===================="
	gcc $(SRCS) `pkg-config --cflags gtk+-2.0` -o $(NAME) $(INCLUDE) $(FLAGS)

clean:
	@echo "\n==================== Cleaning $(NAME) ====================="
	rm -rf *~ \#*\# $(NAME)
