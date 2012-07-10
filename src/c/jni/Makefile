DIRSRC		=	./src/

SRCS		=	$(DIRSRC)../../plugins/comm_inter_process/src/client.c $(DIRSRC)../../utils/utils.c $(DIRSRC)TissueStack.c 

INCLUDE		=	-I./headers -I./../headers -I./../utils -I./../plugins/minc_info/headers -I./../plugins/comm_inter_process/headers

LIB_PATH	=	-L/usr/local/lib

NAME		=	libTissueStack.so
BIN_NAME	=	TissueStack

FLAGS		=	-g3 -Wall -Werror

PLUGIN_INSTALL_DIR = /usr/local/lib

all: compile install

compile:	
	@echo "\n================== Building Shared Library For Tissue Stack JNI Access Layer ===================="
	gcc $(SRCS) -c $(INCLUDE) $(LIB_PATH) $(LIBS) $(FLAGS) -fPIC
	gcc *.o -o $(NAME) $(INCLUDE) $(LIB_PATH) $(LIBS) $(FLAGS) -shared

install:
	echo "\n=========== Copy shared library to: /usr/local/lib ==============="
	mkdir -p $(PLUGIN_INSTALL_DIR)
	cp $(NAME) $(PLUGIN_INSTALL_DIR)/$(NAME)

clean:
	@echo "\n==================== Cleaning $(NAME) ====================="
	@rm -rf *~ \#*\# $(NAME) $(BIN_NAME) *.o
	rm -rf *.o
	rm -rf *.so