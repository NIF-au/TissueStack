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

INCLUDE		=	-I./headers -I./utils -I/usr/include/nifti -I/usr/local/bic/include

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

FLAGS		=	-Wall -Werror -ggdb -O2 -Wno-unused-result

OBJS		=	$(SRCS:%.c=%.o)

CC		=	gcc

# important configuration variables with defaults
export VERSION
export APPLICATION_PATH ?= /opt/tissuestack
# the application path including the version
export APPLICATION_ROOT	=	$(APPLICATION_PATH)/$(VERSION)
# the destination for the binaries
export BINS_PATH			=	$(APPLICATION_ROOT)/bin
# the plugin path with its .so 
export PLUGINS_PATH			=	$(APPLICATION_ROOT)/plugins
# the jni shared library
export JNI_LIB_PATH		=	/usr/local/lib

%.o: %.c
	@echo -e "\tCompiling $(@)."
	@$(CC) -DAPPLICATION_PATH='"$(APPLICATION_ROOT)"' -c $(FLAGS) $(INCLUDE) `pkg-config --cflags gtk+-2.0` -o $@ $<

all:	compile install

prepare:
# the version has to be set for a production build (not debug build)
ifndef VERSION
	$(error VERSION IS NOT SET !!!!!!!!!!!!! Please call make like this: make VERSION=X.X !!!!!)
endif

	@echo -e "\n###################### TISSUE STACK BUILD [Version: $(VERSION)] ###################"
	@echo -e "    APPLICATION ROOT:   $(APPLICATION_ROOT)"
	@echo -e "    PLUGINS PATH:       $(PLUGINS_PATH)"
	@echo -e "    BINARIES PATH:      $(BINS_PATH)"
	@echo -e "    JNI LIB PATH:       $(JNI_LIB_PATH)"
	@echo -e "###################### TISSUE STACK BUILD [Version: $(VERSION)] ###################\n"
	
coreCompile:	$(OBJS)
	@$(CC) $(OBJS) -o $(NAME) $(INCLUDE) $(LIB_PATH) $(LIBS) $(FLAGS) `pkg-config --cflags --libs gtk+-2.0`
	@echo -e "\tFinished compiling core components."

pluginsCompile:
		@echo -e "\n\n\tCompiling plugins:"
		@echo -e "\t-------------------\n"
		@rm -rf volume.o
		@rm -rf utils/utils.o
		@make --no-print-directory -C ./plugins/communicator/ compile 
		@rm -rf volume.o
		@rm -rf utils/utils.o	
		@make --no-print-directory -C ./plugins/image_extract/ compile
		@rm -rf volume.o
		@rm -rf utils/utils.o	
		@make --no-print-directory -C ./plugins/comm_inter_process/ compile
		@rm -rf volume.o
		@rm -rf utils/utils.o	
		@make --no-print-directory -C ./plugins/minc_info/ compile
		@rm -rf volume.o
		@rm -rf utils/utils.o	
		@make --no-print-directory -C ./plugins/minc_convert/ compile
		@rm -rf volume.o
		@rm -rf utils/utils.o	
		@make --no-print-directory -C ./plugins/nifti_convert/ compile
		@rm -rf volume.o
		@rm -rf utils/utils.o
		@make --no-print-directory -C ./plugins/percent/ compile
		@rm -rf volume.o
		@rm -rf utils/utils.o
		@make --no-print-directory -C ./jni/ compile
		@echo -e "\n\tFinished compiling plugins."

install: prepare
	@echo -e "\n\tInstalling Binaries:"
	@echo -e "\t---------------------\n"
	@echo -e "\tInstalling '$(NAME)' executable into: $(BINS_PATH)."
	@mkdir -p $(BINS_PATH)
	@cp $(NAME) $(BINS_PATH)/$(NAME)
	@echo -e "\tFinished installing '$(NAME)'."
	@make --no-print-directory -C ./plugins/communicator/ install 
	@make --no-print-directory -C ./plugins/image_extract/ install
	@make --no-print-directory -C ./plugins/comm_inter_process/ install
	@make --no-print-directory -C ./plugins/minc_convert/ install
	@make --no-print-directory -C ./plugins/nifti_convert/ install
	@make --no-print-directory -C ./plugins/minc_info/ install
	@make --no-print-directory -C ./plugins/percent/ install
	@make --no-print-directory -C ./jni/ install
	@echo -e "\n\tFinished installing binaries.\n"	

clean:
	@echo -e "\n\tCleaning $(NAME)...\n"
	@rm -rf *~ \#*\# $(NAME) ./png/* core hs_err_pid*
	@rm -rf $(DIRSRC)*.o
	@make --no-print-directory -C ./utils/ clean
	@make --no-print-directory -C ./plugins/communicator/ clean
	@make --no-print-directory -C ./plugins/image_extract/ clean
	@make --no-print-directory -C ./plugins/comm_inter_process/ clean
	@make --no-print-directory -C ./plugins/minc_info/ clean
	@make --no-print-directory -C ./plugins/minc_convert/ clean
	@make --no-print-directory -C ./plugins/nifti_convert/ clean
	@make --no-print-directory -C ./plugins/percent/ clean
	@make --no-print-directory -C ./jni/ clean
	@echo -e "\tFinished cleaning $(NAME).\n"

debug: FLAGS += -D_TSS_DEBUG_

debug: coreCompile pluginsCompile

compile: prepare coreCompile pluginsCompile

recompile: clean compile

tomcatstop:
	/opt/apache-tomcat-7.0.27/bin/shutdown.sh

tomcatstart:
	/opt/apache-tomcat-7.0.27/bin/startup.sh

re: tomcatstop clean debug install tomcatstart

.PHONY: all prepare compile install clean re recompile
