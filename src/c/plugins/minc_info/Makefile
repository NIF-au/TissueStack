DIRSRC		=	./src/

SRCS		=	$(DIRSRC)minc_info.c ./../../utils/utils.c

INCLUDE		=	-I./headers -I./../../headers -I./../../utils -I/usr/local/bic/include

LIB_PATH	=	

NAME		=	TissueStackMincInfo.so

FLAGS		=	-ggdb -O2 -Wall -Werror -Wno-unused-result

PLUGIN_INSTALL_DIR = /usr/local/plugins

OBJS		=	$(SRCS:%.c=%.o)

%.o: %.c
	@echo -e "\tCompiling $(@)."
	@gcc -c $(FLAGS) $(INCLUDE) `pkg-config --cflags gtk+-2.0` -fPIC -o $@ $<

all: compile install

compile: $(OBJS)
	@gcc $(OBJS) -o $(NAME) $(LIB_PATH) $(LIBS) $(FLAGS) -shared
	@echo -e "\tFinished building $(NAME).\n"	

clean:
	@echo -e "\tCleaning $(NAME)..."
	@rm -rf *~ \#*\# $(NAME) ./png/*
	@rm -rf $(DIRSRC)*.o
	@rm -rf *.so
	@rm -rf core
	@echo -e "\tFinished cleaning $(NAME).\n"

install:
	@echo -e "\n\tInstalling shared library '$(NAME)' into: $(PLUGINS_PATH)."
	@mkdir -p $(PLUGINS_PATH)
	@cp $(NAME) $(PLUGINS_PATH)/$(NAME)
	@echo -e "\tFinished installing shared library '$(NAME)'."

re:	clean all

.PHONY: all compile install clean re
