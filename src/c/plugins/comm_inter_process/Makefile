DIRSRC		=	./src/

SRCS		=	$(DIRSRC)serv.c

INCLUDE		=	-I./headers -I../../headers -I./../../utils -I/usr/local/bic/include

LIB_PATH	=	

LIBS		=	-lhdf5								\
			-lnetcdf 							\
			-lminc2								\
			-lpthread							\
			-ldl

NAME		=	TissueStackProcessCommunicator.so

FLAGS		=	-ggdb -O2 -Wall -Werror -Wno-unused-result

OBJS		=	$(SRCS:%.c=%.o)

%.o: %.c
	@echo -e "\tCompiling $(@)."
	@gcc -c $(FLAGS) $(INCLUDE) `pkg-config --cflags gtk+-2.0` -fPIC -o $@ $<

all: compile install

compile: $(OBJS)
	@gcc $(OBJS) -o $(NAME) $(LIB_PATH) $(LIBS) $(FLAGS) -shared
	@echo -e "\tFinished building $(NAME).\n"

install:
	@echo -e "\n\tInstalling shared library '$(NAME)' into: $(PLUGINS_PATH)."
	@mkdir -p $(PLUGINS_PATH)
	@cp $(NAME) $(PLUGINS_PATH)/$(NAME)
	@echo -e "\tFinished installing shared library '$(NAME)'."

clean:
	@echo -e "\tCleaning $(NAME)..."
	@rm -rf *~ \#*\# $(NAME) ./png/*
	@rm -rf $(DIRSRC)*.o
	@rm -rf *.so
	@rm -rf core
	@echo -e "\tFinished cleaning $(NAME).\n"

re: clean all

.PHONY: all compile install clean re
