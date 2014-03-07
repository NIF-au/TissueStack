DIRSRC		=	./src/

SRCS		=	$(DIRSRC)serv.c	$(DIRSRC)../../../volume.c $(DIRSRC)../../../utils/utils.c

INCLUDE		=	-I/usr/include/nifti -I./headers -I../../headers -I./../../utils

LIB_PATH	=	

LIBS		= -lpthread							\
			-ldl								\
			-lm

NAME		=	TissueStackCommunicator.so

FLAGS		=	-ggdb -O2 -Wall -Werror -Wno-unused-result

OBJS		=	$(SRCS:%.c=%.o)

%.o: %.c
	@echo -e "\tCompiling $(@)."
	@gcc -DAPPLICATION_PATH='"$(DATA_PATH)"' -DPLUGINS_PATH='"$(PLUGINS_PATH)"' $(APP_FLAGS) \
	-c $(FLAGS) $(INCLUDE) `pkg-config --cflags gtk+-2.0` `GraphicsMagick-config --cppflags --libs --ldflags` -fPIC -o $@ $<

all:	compile install

compile: $(OBJS)
	@gcc $(OBJS) -o $(NAME) $(LIB_PATH) $(LIBS) $(FLAGS) -shared
	@echo -e "\tFinished building $(NAME).\n"

install:
	@echo -e "\n\tInstalling shared library '$(NAME)' into: $(PLUGINS_PATH)."
	@sudo mkdir -p $(PLUGINS_PATH)
	@sudo cp $(NAME) $(PLUGINS_PATH)/$(NAME)
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
