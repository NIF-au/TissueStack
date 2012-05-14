DIRSRC		=	./

SRCS		=	$(DIRSRC)minc_extract_png.c					\
			$(DIRSRC)slice.c						\
			$(DIRSRC)png_creator.c						\
			$(DIRSRC)thread_pool.c

INCLUDE		=	-I/home/oliver/workspace/minc-2.1.10/libsrc2 -I./headers

LIB_PATH	=	-L/home/oliver/workspace/minc-2.1.10/.libs

LIBS		=	-lhdf5								\
			-lnetcdf 							\
			-lminc2								\
			-lpng								\
			-lpthread

NAME		=	yop

FLAGS		=	-g -Wall -Werror -DDEB

all:	
	@echo "\n================== Compilation ===================="
	@gcc $(SRCS) -o $(NAME) $(INCLUDE) $(LIB_PATH) $(LIBS) $(FLAGS)

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