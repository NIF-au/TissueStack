DIRSRC		=	./src/

SRCS		=	$(DIRSRC)minc_info.c ./../../utils/utils.c

INCLUDE		=	-I./headers -I./../../headers -I./../../utils

LIB_PATH	=	

NAME		=	TissueStackMincInfo.so

FLAGS		=	-g3 -Wall -Werror

PLUGIN_INSTALL_DIR = /usr/local/plugins

all: compile install

compile:	
	@echo "\n================== Compile $(NAME) ===================="
	gcc $(SRCS) -c $(INCLUDE) $(FLAGS) -fPIC
	gcc *.o -o $(NAME) $(INCLUDE) $(LIB_PATH) $(LIBS) $(FLAGS) -shared

install:
	mkdir -p $(PLUGIN_INSTALL_DIR)
	cp $(NAME) $(PLUGIN_INSTALL_DIR)/$(NAME)


clean:
	@echo "\n==================== Cleaning $(NAME) ====================="
	rm -rf *~ \#*\# $(NAME)
	rm -rf *.o
	rm -rf *.so
	rm -rf core
