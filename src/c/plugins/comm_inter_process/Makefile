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

PLUGIN_INSTALL_DIR = /usr/local/plugins

OBJS		=	$(SRCS:%.c=%.o)

%.o: %.c
	gcc -c $(FLAGS) $(INCLUDE) `pkg-config --cflags gtk+-2.0` -fPIC -o $@ $<

all: compile install

compile: $(OBJS)
	@echo "\n================== Compile $(NAME) ===================="
	gcc $(OBJS) -o $(NAME) $(LIB_PATH) $(LIBS) $(FLAGS) -shared

install:
	mkdir -p $(PLUGIN_INSTALL_DIR)
	rm -rf $(PLUGIN_INSTALL_DIR)/$(NAME)
	cp $(NAME) $(PLUGIN_INSTALL_DIR)/$(NAME)

clean:
	@echo "\n==================== Cleaning $(NAME) ====================="
	rm -rf *~ \#*\# $(NAME) ./png/*
	rm -rf $(DIRSRC)*.o
	rm -rf *.so
	rm -rf core

re: clean all

.PHONY: all compile install clean re
