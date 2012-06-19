DIRSRC		=	./src/

SRCS		=	$(DIRSRC)minc_extract_png.c					\
			$(DIRSRC)png_creator.c						\
			$(DIRSRC)slice.c

INCLUDE		=	-I./headers -I../../../headers

LIB_PATH	=	

LIBS		=	-lhdf5								\
			-lnetcdf 							\
			-lminc2								\
			-lpng								\
			-lpthread							\
			-ldl

NAME		=	TissueStackPngExtract.so

FLAGS		=	-g3 -Wall -Werror -DDEB

all:	
	@echo "\n================== Compilation ===================="
	@gcc $(SRCS) -c $(INCLUDE) $(FLAGS) -fPIC
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