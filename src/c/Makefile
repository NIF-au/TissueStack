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

USE_MEMORY_MAPPING	=	no
APP_FLAGS			=

# important configuration variables with defaults
export VERSION
export APPLICATION_PATH ?= /usr/local/tissuestack
export DATA_PATH ?= /opt/tissuestack
# the application path including the version
export APPLICATION_ROOT	=	$(APPLICATION_PATH)/$(VERSION)
# the destination for the binaries
export BINS_PATH			=	$(APPLICATION_ROOT)/bin
# the plugin path with its .so 
export PLUGINS_PATH			=	$(APPLICATION_ROOT)/plugins
# the jni shared library
export JNI_LIB_PATH		=	/usr/local/lib
# use memory mapping (turned off by default. did not really deliver gains in our case but can be turned on again with USE_MM=true)?
ifdef USE_MM
ifeq ($(USE_MM),true)
APP_FLAGS=-DUSE_MM
USE_MEMORY_MAPPING=yes
endif
endif

#build path for binary distribution
export PROFILE_PATH		= 	/etc/profile.d
export BASE_DIR			= 	$(DIRSRC)../../
export BUILD_PATH		=	/tmp/tissuestack_build
export BUILD_SRC_PATH		=	$(BUILD_PATH)/src
export DIST_NAME		=	tissuestack-$(VERSION).tar.gz
export CURRENT_BRANCH	=	$(shell git branch | grep "*" | sed "s/* //";)

%.o: %.c
	@echo -e "\tCompiling $(@)."
	@$(CC) -DAPPLICATION_PATH='"$(DATA_PATH)"' -DPLUGINS_PATH='"$(PLUGINS_PATH)"' $(APP_FLAGS) \
		-c $(FLAGS) $(INCLUDE) `pkg-config --cflags gtk+-2.0` -o $@ $<

all:	compile install

prepare:
# the version has to be set for a production build (not debug build)
ifndef VERSION
	$(error VERSION IS NOT SET !!!!!!!!!!!!! Please call make like this: make VERSION=X.X !!!!!)
endif

	@echo -e "\n###################### TISSUE STACK BUILD [Version: $(VERSION)] ###################"
	@echo -e "    APPLICATION DATA:      $(DATA_PATH)"
	@echo -e "    PLUGINS PATH:          $(PLUGINS_PATH)"
	@echo -e "    BINARIES PATH:         $(BINS_PATH)"
	@echo -e "    JNI LIB PATH:          $(JNI_LIB_PATH)"
	@echo -e "    USE MEMORY MAPPING:    $(USE_MEMORY_MAPPING)"
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
	@echo -e "\n\tInstalling Binaries (required super user priviledges):"
	@echo -e "\t-----------------------------------------------\n"
	@echo -e "\tInstalling '$(NAME)' executable into: $(BINS_PATH)."
	@sudo mkdir -p $(BINS_PATH)
	@sudo cp $(NAME) $(BINS_PATH)/$(NAME)
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

# target for building a binary tar.gz (is then used for rpm build)
# we depend on target 'all' and use a temporary build directory in /tmp/tissuestack_build
dist: all
	@echo -e "\n\n\tBuilding binary archive: $(DIST_NAME):"
	@echo -e "\t---------------------------------------------------"
	
	@echo -e "\n\tPreparing build directory..."
	@rm -rf $(BUILD_PATH)
	@mkdir -p $(BUILD_PATH)/logs
	@mkdir -p $(BUILD_SRC_PATH)$(PLUGINS_PATH)
	@mkdir -p $(BUILD_SRC_PATH)$(BINS_PATH)
	@mkdir -p $(BUILD_SRC_PATH)$(JNI_LIB_PATH)
	@mkdir -p $(BUILD_SRC_PATH)$(PROFILE_PATH)
	@mkdir -p $(BUILD_SRC_PATH)$(DATA_PATH)/ands
	@mkdir -p $(BUILD_SRC_PATH)$(DATA_PATH)/bin
	@mkdir -p $(BUILD_SRC_PATH)$(DATA_PATH)/colormaps
	@mkdir -p $(BUILD_SRC_PATH)$(DATA_PATH)/data
	@mkdir -p $(BUILD_SRC_PATH)$(DATA_PATH)/logs
	@mkdir -p $(BUILD_SRC_PATH)$(DATA_PATH)/lookup
	@mkdir -p $(BUILD_SRC_PATH)$(DATA_PATH)/tasks
	@mkdir -p $(BUILD_SRC_PATH)$(DATA_PATH)/tiles
	@mkdir -p $(BUILD_SRC_PATH)$(DATA_PATH)/upload

	@echo -e "\tCopying over Tissue Stack Image Server executables and shared libraries..."
	@cp $(BINS_PATH)/* $(BUILD_SRC_PATH)$(BINS_PATH)
	@cp $(PLUGINS_PATH)/*.so $(BUILD_SRC_PATH)$(PLUGINS_PATH)
	@cp $(JNI_LIB_PATH)/libTissueStack.so.$(VERSION) $(BUILD_SRC_PATH)$(JNI_LIB_PATH)
	@ln -s $(JNI_LIB_PATH)/libTissueStack.so.$(VERSION) $(BUILD_SRC_PATH)$(JNI_LIB_PATH)/libTissueStack.so
	
	@echo -e "\tCopying over front-end resources and colormaps..."
	@cp $(BASE_DIR)data/colormaps/* $(BUILD_SRC_PATH)$(DATA_PATH)/colormaps
	@cp -r $(BASE_DIR)/src/web $(BUILD_SRC_PATH)$(DATA_PATH)
	@ln -s $(DATA_PATH)/tiles $(BUILD_SRC_PATH)$(DATA_PATH)/web/tiles
	@rm -rf $(BUILD_PATH)/logs
	@mkdir -p $(BUILD_PATH)/logs
	
	@echo -e "\tSwitching to 'packaging' branch to copy over java runtime [error logs: $(BUILD_PATH)/logs/*.log]..."
	@cd $(BASE_DIR); git checkout packaging 2>$(BUILD_PATH)/logs/tissuestack_copy.log; sleep 1s; \
	cp -r TissueStack/tissuestack/jdk1.6.0_25 $(BUILD_SRC_PATH)$(DATA_PATH) >> $(BUILD_PATH)/logs/tissuestack_copy.log || \
	(git checkout $(CURRENT_BRANCH) >> $(BUILD_PATH)/logs/tissuestack_copy.log && echo "ERROR: git error => check logs !!!" && exit -1);

	@echo -e "\tSwitching to 'packaging' branch to copy over tomcat binaries [error logs: $(BUILD_PATH)/logs/*.log]..."
	@cd $(BASE_DIR); git checkout packaging 2>$(BUILD_PATH)/logs/tissuestack_copy.log; sleep 1s; \
	cp -r TissueStack/tissuestack/apache-tomcat-7.0.35 $(BUILD_SRC_PATH)$(DATA_PATH) >> $(BUILD_PATH)/logs/tissuestack_copy.log || \
	(git checkout $(CURRENT_BRANCH) 2> $(BUILD_PATH)/logs/tissuestack_copy.log && echo "ERROR: git error => check logs !!!" && exit -1);

	@echo -e "\tSwitching to 'packaging' branch to copy over tissue stack start scripts [error logs: $(BUILD_PATH)/logs/*.log]..."
	@cd $(BASE_DIR); git checkout packaging 2>$(BUILD_PATH)/logs/tissuestack_copy.log; sleep 1s; \
	cp -r TissueStack/tissuestack/bin/tissuestack.sh $(BUILD_SRC_PATH)$(BINS_PATH)/tissuestack >> $(BUILD_PATH)/logs/tissuestack_copy.log || \
	(git checkout $(CURRENT_BRANCH) 2> $(BUILD_PATH)/logs/tissuestack_copy.log && echo "ERROR: git error => check logs !!!" && exit -1);
	chmod 755 $(BUILD_SRC_PATH)$(BINS_PATH)/tissuestack

	@echo -e "\tCreating environament script containing the desired paths..."
	@sudo sh -c "echo -e '#!/bin/bash\n\
	TISSUE_STACK_HOME=$(DATA_PATH)\n\
	JAVA_HOME=\$$TISSUE_STACK_HOME/jdk1.6.0_25\n\
	CATALINA_HOME=\$$TISSUE_STACK_HOME/apache-tomcat-7.0.35\n\
	PATH=\$$PATH:$(BINS_PATH)\n\
	export TISSUE_STACK_HOME JAVA_HOME CATALINA_HOME PATH' > $(BUILD_SRC_PATH)$(PROFILE_PATH)/tissuestack_env.sh";
	@sudo chmod 644 $(BUILD_SRC_PATH)$(PROFILE_PATH)/tissuestack_env.sh

	@echo -e "\tBuilding Tissue Stack Web Service (maven build). This may take a while, please be patient! [error logs: $(BUILD_PATH)/logs/*.log]..."
	@cd $(BASE_DIR); git checkout $(CURRENT_BRANCH) &>$(BUILD_PATH)/logs/tissuestack_copy.log || (echo "ERROR: git error => check logs !!!" && exit -1)
	@cd $(BASE_DIR);	mvn clean install > $(BUILD_PATH)/logs/tissuestack_maven_build.log
	@echo -e "\tCopying over war file into tomcat web application directory..."
	@cp $(BASE_DIR)target/TissueStack.war $(BUILD_SRC_PATH)$(DATA_PATH)/apache-tomcat-7.0.35/webapps
	
	
#TODO: cp of bash scripts for start
#TODO: create update sql with changed paths
	
	@echo -e "\tZipping up everything [log: $(BUILD_PATH)/logs/tissuestack_tar.log ]..."
	@cd $(BUILD_SRC_PATH); tar cvzf $(BUILD_PATH)/$(DIST_NAME) * &> $(BUILD_PATH)/logs/tissuestack_tar.log
	
	@echo -e "\n\tFinished creating binary archive:  $(BUILD_PATH)/$(DIST_NAME)"
		
.PHONY: all prepare compile install clean re recompile
