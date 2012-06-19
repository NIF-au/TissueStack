DIRSRC		=	./

SRCS		=	$(DIRSRC)core.c							\
			$(DIRSRC)prompt.c						\
			$(DIRSRC)plugin.c						\
			$(DIRSRC)volume.c						\
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


all:	
	@echo "\n================== Compilation ===================="
	@gcc $(SRCS) -o $(NAME) $(INCLUDE) $(LIB_PATH) $(LIBS) $(FLAGS)

general: all plug install

plug:
	@echo "\n======== Compilation Communicator Plugin =========="
	@make -C ./plugins/communicator/ re
	@echo "\n======== Compilation PNGExtractor Plugin =========="
	@make -C ./plugins/png_extract/ re
	@echo "\n===== Compilation ProcessCommunicator Plugin ======"
	@make -C ./plugins/comm_inter_process/ re

install:
	@echo "\n============= Installing Binaries ================="
	@mkdir -p /usr/local/bin/
	@mkdir -p /usr/local/plugins/
	@mv ./TissueStackImageServer /usr/local/bin/
	@mv ./plugins/communicator/TissueStackCommunicator.so /usr/local/plugins/	
	@mv ./plugins/png_extract/TissueStackPNGExtract.so /usr/local/plugins/
	@mv ./plugins/comm_inter_process/TissueStackProcessCommunicator.so /usr/local/plugins/

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
