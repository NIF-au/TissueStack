DIRSRC		=	./src/

SRCS		=	$(DIRSRC)minc_info.c

INCLUDE		=	-I./headers -I./../../headers -I/usr/local/include

LIB_PATH	=	-L/usr/local/lib

LIBS		=	-lhdf5								\
			-lnetcdf 							\
			-lminc2								\
			-lvolume_io2
			
NAME		=	libTissueStackMincInfo.so

FLAGS		=	-g3 -Wall -Werror

all: lib copyToUsrLocalLib

lib:
	@echo "\n================== Shared Library Compilation ===================="
	@gcc $(SRCS) -c $(INCLUDE) $(LIB_PATH) $(LIBS) $(FLAGS) -fPIC
	@gcc *.o -o $(NAME) $(INCLUDE) $(LIB_PATH) $(LIBS) $(FLAGS) -shared

copyToUsrLocalLib:
	@echo "\n=========== Copy shared library to: /usr/local/lib ==============="
	@cp $(NAME) /usr/local/lib
	
clean:
	@echo "\n==================== Cleaning ====================="
	@rm -rf *~ \#*\# $(NAME) *.o