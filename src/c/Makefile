DIRSRC		=	./

SRCS		=	$(DIRSRC)core.c							\
			$(DIRSRC)prompt.c						\
			$(DIRSRC)plugin.c						\
			$(DIRSRC)volume.c						\
			$(DIRSRC)error.c						\
			$(DIRSRC)thread_pool.c

INCLUDE		=	-I./headers 

LIB_PATH	=	

LIBS		=	-lhdf5								\
			-lnetcdf 							\
			-lminc2								\
			-lpng								\
			-lpthread							\
			-ldl

NAME		=	TissueStackImageServer

FLAGS		=	-Wall -Werror -DDEB -g3


all: compile install

compile:	
	@clear
	@echo "\n================== Compile $(NAME) ===================="
	gcc $(SRCS) -o $(NAME) $(INCLUDE) $(LIB_PATH) $(LIBS) $(FLAGS)
	make -C ./plugins/communicator/ compile
	make -C ./plugins/png_extract/ compile
	make -C ./plugins/comm_inter_process/ compile
	make -C ./plugins/minc_info/ compile
	make -C ./jni/ compile

install:
	@echo "\n============= Installing Binaries ================="
	mkdir -p /usr/local/bin/
	cp $(NAME) /usr/local/bin/
	make -C ./plugins/communicator/ install
	make -C ./plugins/png_extract/ install
	make -C ./plugins/comm_inter_process/ install
	make -C ./plugins/minc_info/ install
	make -C ./jni/ install

clean:
	@echo "\n==================== Cleaning $(NAME) ====================="
	rm -rf *~ \#*\# $(NAME) ./png/* core
	rm -rf *.o
	make -C ./plugins/communicator/ clean
	make -C ./plugins/png_extract/ clean
	make -C ./plugins/comm_inter_process/ clean
	make -C ./plugins/minc_info/ clean
	make -C ./jni/ clean

e:	clean
	@echo "\n==================== Execution ====================\n\n"
	@./yop ../00-normal-model-nonsym.mnc
	@echo "\n\n================== Exec finished ==================\n\n"

time:	clean
	@echo "\n==================== Execution ====================\n\n"
	@time ./yop ../00-normal-model-nonsym.mnc
	@echo "\n\n================== Exec finished ==================\n\n"
