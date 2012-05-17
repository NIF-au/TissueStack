DIRSRC		=	./src/

SRCS		=	$(DIRSRC)minc_extract_png.c					\
			$(DIRSRC)png_creator.c						\
			$(DIRSRC)slice.c

INCLUDE		=	-I/home/oliver/workspace/minc-2.1.10/libsrc2 -I./headers -I../../../headers

LIB_PATH	=	-L/home/oliver/workspace/minc-2.1.10/.libs

LIBS		=	-lhdf5								\
			-lnetcdf 							\
			-lminc2								\
			-lpng								\
			-lpthread							\
			-ldl

NAME		=	yop.so

FLAGS		=	-g3 -Wall -Werror -DDEB

all:	
	@echo "\n================== Compilation ===================="
	@gcc $(SRCS) -c $(INCLUDE) $(LIB_PATH) $(LIBS) $(FLAGS) -fPIC
	@gcc *.o -o $(NAME) $(INCLUDE) $(LIB_PATH) $(LIBS) $(FLAGS) -shared

clean:
	@echo "\n==================== Cleaning ====================="
	@rm -rf *~ \#*\# $(NAME) ./png/*

fclean:	clean
	@rm -rf *.o

re:	fclean all

e:	re
	@echo "\n==================== Execution ====================\n\n"
	@./yop ../00-normal-model-nonsym.mnc
	@echo "\n\n================== Exec finished ==================\n\n"

time:	re
	@echo "\n==================== Execution ====================\n\n"
	@time ./yop ../00-normal-model-nonsym.mnc
	@echo "\n\n================== Exec finished ==================\n\n"