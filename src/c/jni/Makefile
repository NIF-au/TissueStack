DIRSRC		=	./src/

SRCS		=	$(DIRSRC)TissueStack.c

INCLUDE		=	-I./headers -I./../headers -I./../plugins/minc_info/headers -I/usr/local/include

LIB_PATH	=	-L/usr/local/lib

LIBS		=	-lhdf5								\
			-lnetcdf 							\
			-lminc2								\
			-lTissueStackMincInfo

NAME		=	libTissueStack.so
BIN_NAME	=	TissueStack

FLAGS		=	-g3 -Wall -Werror

all: bin lib copyToUsrLocalLib

lib:
	@echo "\n================== Shared Library Compilation ===================="
	@gcc $(SRCS) -c $(INCLUDE) $(LIB_PATH) $(LIBS) $(FLAGS) -fPIC
	@gcc *.o -o $(NAME) $(INCLUDE) $(LIB_PATH) $(LIBS) $(FLAGS) -shared
	
copyToUsrLocalLib:
	@echo "\n=========== Copy shared library to: /usr/local/lib ==============="
	@cp $(NAME) /usr/local/lib

bin:
	@echo "\n==================      Binary Compilation     ===================="
	@gcc $(SRCS) -c $(INCLUDE) $(LIB_PATH) $(LIBS) $(FLAGS)
	@gcc *.o -o $(BIN_NAME) $(INCLUDE) $(LIB_PATH) $(LIBS) $(FLAGS)

clean:
	@echo "\n==================== Cleaning ====================="
	@rm -rf *~ \#*\# $(NAME) $(BIN_NAME) *.o