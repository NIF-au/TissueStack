DIRSRC		=	./src/

SRCS		=	$(DIRSRC)../../plugins/comm_inter_process/src/client.c $(DIRSRC)../../utils/utils.c $(DIRSRC)TissueStack.c 

INCLUDE		=	-I./headers -I./../headers -I./../utils -I./../plugins/minc_info/headers -I./../plugins/comm_inter_process/headers

LIB_PATH	=	-L/usr/local/lib

NAME		=	libTissueStack.so
BIN_NAME	=	TissueStack

FLAGS		=	-ggdb -O2 -lm -Wall -Werror -Wno-unused-result

SHARED_LIBS_DIR = /usr/local/lib

OBJS		=	$(SRCS:%.c=%.o)

%.o: %.c
	@echo -e "\tCompiling $(@)."
	@gcc -DAPPLICATION_PATH='"$(DATA_PATH)"' -DPLUGINS_PATH='"$(PLUGINS_PATH)"' $(APP_FLAGS) \
	-c $(FLAGS) $(INCLUDE) `GraphicsMagick-config --cppflags` `pkg-config --cflags gtk+-2.0` -fPIC -o $@ $<

all: compile install

compile: $(OBJS)
	@gcc $(OBJS) -o $(NAME) $(LIB_PATH) $(LIBS) $(FLAGS) -shared
	@echo -e "\tFinished building $(NAME).\n"

install:
	@echo -e "\n\tInstalling shared library '$(NAME)' into: $(JNI_LIB_PATH)."
	@sudo mkdir -p $(JNI_LIB_PATH)
	@sudo cp $(NAME) $(JNI_LIB_PATH)/$(NAME).$(VERSION)
	@echo -e "\tCreating symbolic link to it: $(SHARED_LIBS_DIR)/$(NAME) => $(JNI_LIB_PATH)/$(NAME).$(VERSION)"
	@sudo ln -sf $(JNI_LIB_PATH)/$(NAME).$(VERSION) $(SHARED_LIBS_DIR)/$(NAME)
	@echo -e "\tFinished installing shared library '$(NAME)'."

clean:
	@echo -e "\tCleaning $(NAME)..."
	@rm -rf *~ \#*\# $(NAME) $(BIN_NAME) *.o
	@rm -rf $(DIRSRC)*.o
	@rm -rf *.so
	@echo -e "\tFinished cleaning $(NAME).\n"
	
re:	clean all

.PHONY: all compile install clean re
