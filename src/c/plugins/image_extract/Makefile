DIRSRC		=	./src/

SRCS		=	$(DIRSRC)image_extract.c					\
			$(DIRSRC)tile_creator.c						\
			$(DIRSRC)slice.c							\
			$(DIRSRC)../../../utils/utils.c

INCLUDE		=	-I./headers -I./../../utils -I../../headers

LIB_PATH	=	

LIBS		=	-lhdf5								\
			-lnetcdf 							\
			-lminc2								\
			-lpthread							\
			-ldl

NAME		=	TissueStackImageExtract.so

FLAGS		=	-g3 -Wall -Werror -DDEB

PLUGIN_INSTALL_DIR = /usr/local/plugins

all: compile install	

compile:
	@echo "\n================== Compile $(NAME) ===================="
	gcc $(SRCS) -c $(INCLUDE) $(FLAGS) -fPIC `GraphicsMagick-config --cppflags` `pkg-config --cflags gtk+-2.0`
	gcc *.o -o $(NAME) $(INCLUDE) $(LIB_PATH) $(LIBS) $(FLAGS) -shared 	\
		`GraphicsMagick-config --ldflags --libs`				\
		`pkg-config --libs gtk+-2.0`

clean:
	@echo "\n==================== Cleaning $(NAME) ====================="
	rm -rf *~ \#*\# $(NAME) ./png/*
	rm -rf *.o
	rm -rf *.so
	rm -rf core

install:
	mkdir -p $(PLUGIN_INSTALL_DIR)
	cp $(NAME) $(PLUGIN_INSTALL_DIR)/$(NAME)

e:	clean
	@echo "\n==================== Execution ====================\n\n"
	@./yop ../00-normal-model-nonsym.mnc
	@echo "\n\n================== Exec finished ==================\n\n"

time:	clean
	@echo "\n==================== Execution ====================\n\n"
	@time ./yop ../00-normal-model-nonsym.mnc
	@echo "\n\n================== Exec finished ==================\n\n"
