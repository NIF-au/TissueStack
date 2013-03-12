DIRSRC		=	./src/

SRCS		=	$(DIRSRC)image_extract.c					\
			$(DIRSRC)tile_creator.c						\
			$(DIRSRC)slice.c							\
			$(DIRSRC)../../../volume.c				\
			$(DIRSRC)../../../utils/utils.c

INCLUDE		=	-I./headers -I./../../utils -I../../headers -I/usr/include/GraphicsMagick -I/usr/local/bic/include

LIB_PATH	=	

LIBS		=	-lhdf5								\
			-lnetcdf 							\
			-lminc2								\
			-lpthread							\
			-ldl

NAME		=	TissueStackImageExtract.so

FLAGS		=	-ggdb -O2 -Wall -Werror -Wno-unused-result

OBJS		=	$(SRCS:%.c=%.o)

%.o: %.c
	@echo -e "\tCompiling $(@)."
	@gcc -c $(FLAGS) `pkg-config --cflags gtk+-2.0` -fPIC -o $@ $< $(INCLUDE)

all: compile install

compile: $(OBJS)
	@gcc $(OBJS) -o $(NAME) $(LIB_PATH) $(LIBS) $(FLAGS) -shared `GraphicsMagick-config --cppflags --libs --ldflags`
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
