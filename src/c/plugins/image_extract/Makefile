DIRSRC		=	./src/

SRCS		=	$(DIRSRC)image_extract.c					\
			$(DIRSRC)tile_creator.c						\
			$(DIRSRC)slice.c							\
			$(DIRSRC)../../../volume.c				\
			$(DIRSRC)../../../utils/utils.c

INCLUDE		=	-I./headers -I./../../utils -I../../headers -I/usr/include/GraphicsMagick

LIB_PATH	=	

LIBS		=	-lhdf5								\
			-lnetcdf 							\
			-lminc2								\
			-lpthread							\
			-ldl

NAME		=	TissueStackImageExtract.so

FLAGS		=	-g3 -Wall -Werror -DDEB

PLUGIN_INSTALL_DIR = /usr/local/plugins

OBJS		=	$(SRCS:%.c=%.o)

%.o: %.c
	gcc -c $(FLAGS) `pkg-config --cflags gtk+-2.0` -fPIC -o $@ $< $(INCLUDE)

all: compile install

compile: $(OBJS)
	@echo "\n================== Compile $(NAME) ===================="
	gcc $(OBJS) -o $(NAME) $(LIB_PATH) $(LIBS) $(FLAGS) -shared `GraphicsMagick-config --cppflags --libs --ldflags`

clean:
	@echo "\n==================== Cleaning $(NAME) ====================="
	rm -rf *~ \#*\# $(NAME) ./png/*
	rm -rf $(DIRSRC)*.o
	rm -rf *.so
	rm -rf core

install:
	mkdir -p $(PLUGIN_INSTALL_DIR)
	cp $(NAME) $(PLUGIN_INSTALL_DIR)/$(NAME)

re:	clean all

.PHONY: all compile install clean re
