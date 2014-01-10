DIRSRC		=	./

SRCS		=	$(DIRSRC)utils.c 

INCLUDE		=	-I./ -I../headers

LIB_PATH	=

LIBS		=

NAME		=	utils

FLAGS		=	-ggdb -lm -O2 -Wall -Werror -Wno-unused-result

all: compile

compile:	
	@echo -e "\t Compiling $(NAME)."
	@gcc $(SRCS) `pkg-config --cflags gtk+-2.0` -o $(NAME)  $(INCLUDE) $(LIB_PATH) $(LIBS) $(FLAGS)

clean:
	@echo -e "\tCleaning $(NAME)..."
	@rm -rf *~ \#*\# $(NAME)
	@rm -rf $(NAME).o
	@rm -rf *.so
	@echo -e "\tFinished cleaning $(NAME).\n"
