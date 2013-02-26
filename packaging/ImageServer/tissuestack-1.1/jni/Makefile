DIRSRC		=	./src/

SRCS		=	$(DIRSRC)../../plugins/comm_inter_process/src/client.c $(DIRSRC)../../utils/utils.c $(DIRSRC)TissueStack.c 

INCLUDE		=	-I./headers -I./../headers -I./../utils -I./../plugins/minc_info/headers -I./../plugins/comm_inter_process/headers -I/usr/local/bic/include

LIB_PATH	=	-L/usr/local/lib

NAME		=	libTissueStack.so
BIN_NAME	=	TissueStack

FLAGS		=	-ggdb -O2 -Wall -Werror -Wno-unused-result

PLUGIN_INSTALL_DIR = /usr/local/lib

OBJS		=	$(SRCS:%.c=%.o)

%.o: %.c
	gcc -c $(FLAGS) $(INCLUDE) `GraphicsMagick-config --cppflags` `pkg-config --cflags gtk+-2.0` -fPIC -o $@ $<

all: compile install

compile: $(OBJS)
	@echo "\n================== Building Shared Library For Tissue Stack JNI Access Layer ===================="
	gcc $(OBJS) -o $(NAME) $(LIB_PATH) $(LIBS) $(FLAGS) -shared

install:
	@echo "\n=========== Copy shared library to: /usr/local/lib ==============="
	mkdir -p $(PLUGIN_INSTALL_DIR)
	cp $(NAME) $(PLUGIN_INSTALL_DIR)/$(NAME)

clean:
	@echo "\n==================== Cleaning $(NAME) ====================="
	@rm -rf *~ \#*\# $(NAME) $(BIN_NAME) *.o
	rm -rf $(DIRSRC)*.o
	rm -rf *.so
re:	clean all

.PHONY: all compile install clean re
