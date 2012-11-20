DIRSRC		=	./

SRCS		=	$(DIRSRC)core.c							\
			$(DIRSRC)prompt.c						\
			$(DIRSRC)plugin.c						\
			$(DIRSRC)volume.c						\
			$(DIRSRC)error.c						\
			$(DIRSRC)notification_center.c					\
			$(DIRSRC)log_center.c						\
			$(DIRSRC)thread_pool.c						\
			$(DIRSRC)utils/utils.c						\
			$(DIRSRC)tile_requests.c					\
			$(DIRSRC)percent_and_time.c					\
			$(DIRSRC)tasks.c						\
			$(DIRSRC)memory_mapping.c					\

INCLUDE		=	-I./headers -I./utils -I/usr/include/nifti

LIB_PATH	=	

LIBS		=	-lhdf5								\
			-lnetcdf 							\
			-lminc2								\
			-lpthread							\
			-lniftiio							\
			-ldl								\
			-lniftiio							\
			-lznz								\
			-lz

NAME		=	TissueStackImageServer

FLAGS		=	-Wall -Werror -ggdb -O2 -Wno-unused-result #-D_GNU_SOURCE

OBJS		=	$(SRCS:%.c=%.o)

CC		=	gcc

%.o: %.c
	$(CC) -c $(FLAGS) $(INCLUDE) `pkg-config --cflags gtk+-2.0` -o $@ $<

all: compile install

compile: $(OBJS)
	@echo "\n================== Compile $(NAME) ===================="
	$(CC) $(OBJS) -o $(NAME) $(INCLUDE) $(LIB_PATH) $(LIBS) $(FLAGS) `pkg-config --cflags --libs gtk+-2.0`
	rm -rf volume.o
	rm -rf utils/utils.o
	make -C ./plugins/communicator/ compile 
	rm -rf volume.o
	rm -rf utils/utils.o	
	make -C ./plugins/image_extract/ compile
	rm -rf volume.o
	rm -rf utils/utils.o	
	make -C ./plugins/comm_inter_process/ compile
	rm -rf volume.o
	rm -rf utils/utils.o	
	make -C ./plugins/minc_info/ compile
	rm -rf volume.o
	rm -rf utils/utils.o	
	make -C ./plugins/minc_convert/ compile
	rm -rf volume.o
	rm -rf utils/utils.o	
	make -C ./plugins/nifti_convert/ compile
	rm -rf volume.o
	rm -rf utils/utils.o
	make -C ./plugins/percent/ compile
	rm -rf volume.o
	rm -rf utils/utils.o
	make -C ./jni/ compile

install:
	@echo "\n============= Installing Binaries ================="
	mkdir -p /usr/local/bin/
	rm -rf /usr/local/bin/$(NAME)
	cp $(NAME) /usr/local/bin/
	make -C ./plugins/communicator/ install 
	make -C ./plugins/image_extract/ install
	make -C ./plugins/comm_inter_process/ install
	make -C ./plugins/minc_convert/ install
	make -C ./plugins/nifti_convert/ install
	make -C ./plugins/minc_info/ install
	make -C ./plugins/percent/ install
	make -C ./jni/ install

clean:
	@echo "\n==================== Cleaning $(NAME) ====================="
	rm -rf *~ \#*\# $(NAME) ./png/* core hs_err_pid*
	rm -rf $(DIRSRC)*.o
	make -C ./utils/ clean
	make -C ./plugins/communicator/ clean
	make -C ./plugins/image_extract/ clean
	make -C ./plugins/comm_inter_process/ clean
	make -C ./plugins/minc_info/ clean
	make -C ./plugins/minc_convert/ clean
	make -C ./plugins/nifti_convert/ clean
	make -C ./plugins/percent/ clean
	make -C ./jni/ clean

recompile: clean compile

tomcatstop:
	/opt/apache-tomcat-7.0.27/bin/shutdown.sh

tomcatstart:
	/opt/apache-tomcat-7.0.27/bin/startup.sh

re: tomcatstop clean all tomcatstart

.PHONY: all compile install clean re recompile
