DIRSRC		=	./src/

SRCS		=	$(DIRSRC)converter.c			\
			$(DIRSRC)../../../utils/utils.c

INCLUDE		=	-I./headers -I./../../headers -I./../../utils -I/usr/local/bic/include -I/usr/local/bic/include

LIB_PATH	=	

LIBS		=	-lhdf5								\
			-lnetcdf 							\
			-lminc2								\
			-lpthread							\
			-ldl								\
			-lm

NAME		=	TissueStackMincConverter.so

FLAGS		=	-Wall -Werror -ggdb -O2 -Wno-unused-result

OBJS		=	$(SRCS:%.c=%.o)

%.o: %.c
	@echo -e "\tCompiling $(@)."
	@gcc -DAPPLICATION_PATH='"$(DATA_PATH)"' -DPLUGINS_PATH='"$(PLUGINS_PATH)"' $(APP_FLAGS) \
	-c $(FLAGS) $(INCLUDE) `GraphicsMagick-config --cppflags` `pkg-config --cflags gtk+-2.0` -fPIC -o $@ $<

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
	@sudo mkdir -p $(PLUGINS_PATH)
	@sudo cp $(NAME) $(PLUGINS_PATH)/$(NAME)
	@echo -e "\tFinished installing shared library '$(NAME)'."

re:	clean all

.PHONY: all compile install clean re
