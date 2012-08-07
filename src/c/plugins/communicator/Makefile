DIRSRC		=	./src/

SRCS		=	$(DIRSRC)serv.c	$(DIRSRC)../../../utils/utils.c

INCLUDE		=	-I./headers -I../../headers -I./../../utils

LIB_PATH	=	

LIBS		=	-lhdf5								\
			-lnetcdf 							\
			-lminc2								\
			-lpthread							\
			-ldl

NAME		=	TissueStackCommunicator.so

FLAGS		=	-g3 -Wall -Werror

PLUGIN_INSTALL_DIR = /usr/local/plugins

all: compile install

compile:	
	@echo "\n================== Compile $(NAME) ===================="
	gcc $(SRCS) -c $(INCLUDE) $(FLAGS) -fPIC `pkg-config --cflags gtk+-2.0`
	gcc *.o -o $(NAME) $(INCLUDE) $(LIB_PATH) $(LIBS) $(FLAGS) -shared `pkg-config --libs gtk+-2.0`

install:
	mkdir -p $(PLUGIN_INSTALL_DIR)
	cp $(NAME) $(PLUGIN_INSTALL_DIR)/$(NAME)

clean:
	@echo "\n==================== Cleaning $(NAME) ====================="
	rm -rf *~ \#*\# $(NAME) ./png/*
	rm -rf *.o
	rm -rf *.so
	rm -rf core

e:	clean
	@echo "\n==================== Execution ====================\n\n"
	@./yop ../00-normal-model-nonsym.mnc
	@echo "\n\n================== Exec finished ==================\n\n"

time:	clean
	@echo "\n==================== Execution ====================\n\n"
	@time ./yop ../00-normal-model-nonsym.mnc
	@echo "\n\n================== Exec finished ==================\n\n"
