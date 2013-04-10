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

EXE_NAME		=	TissueStackImageServer

FLAGS		=	-Wall -Werror -ggdb -O2 -Wno-unused-result

OBJS		=	$(SRCS:%.c=%.o)

CC		=	gcc

USE_MEMORY_MAPPING	=	no
APP_FLAGS			=

# important configuration variables with defaults
export NAME	=	tissuestack
export VERSION
export APPLICATION_PATH ?= /usr/local/$(NAME)
export DATA_PATH ?= /opt/$(NAME)
# the application path including the version
export APPLICATION_ROOT	=	$(APPLICATION_PATH)/$(VERSION)
# the destination for the binaries
export BINS_PATH			=	$(APPLICATION_ROOT)/bin
# the plugin path with its .so 
export PLUGINS_PATH			=	$(APPLICATION_ROOT)/plugins
# the jni shared library
export JNI_LIB_PATH			=	$(APPLICATION_ROOT)/lib
# packaging stuff including java runtime, tomcat and miscalenous configuration files
export PACKAGING_PATH		=	$(BASE_DIR)packaging
export JDK_VERSION			=	1.6.0_25
export JDK_PATH				=	$(PACKAGING_PATH)/jdk$(JDK_VERSION)
export TOMCAT_VERSION		= 	7.0.35
export TOMCAT_PATH			=	$(PACKAGING_PATH)/apache-tomcat-$(TOMCAT_VERSION)
export PACKAGING_CONFIG_PATH=	$(PACKAGING_PATH)/conf
export IS_DEBIAN_BASED=`cat /etc/*-release | grep -i debian | wc -c`
export IS_UBUNTU_BASED=`cat /etc/*-release | grep -i ubuntu | wc -c`
export HTTPD_CONF_DIR		= 	/etc/httpd/conf.d
export APACHE2_CONF_DIR		= 	/etc/apache2/sites-available
export SYS_V_INIT_PATH		= 	/etc/init.d/

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
export BUILD_PATH		=	/tmp/$(NAME)_build
export BUILD_SRC_PATH		=	$(BUILD_PATH)/src
export DIST_NAME		=	$(NAME)-$(VERSION).tar.gz
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
	@$(CC) $(OBJS) -o $(EXE_NAME) $(INCLUDE) $(LIB_PATH) $(LIBS) $(FLAGS) `pkg-config --cflags --libs gtk+-2.0` > /dev/null
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
	@echo -e "\n\tInstalling Binaries (requires super user priviledges):"
	@echo -e "\t-----------------------------------------------\n"
	@echo -e "\tInstalling '$(EXE_NAME)' executable into: $(BINS_PATH)."
	@sudo mkdir -p $(BINS_PATH)
	@sudo cp $(EXE_NAME) $(BINS_PATH)/$(EXE_NAME)
	@echo -e "\tFinished installing '$(EXE_NAME)'."
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
	@mkdir -p $(BUILD_SRC_PATH)$(DATA_PATH)/colormaps
	@mkdir -p $(BUILD_SRC_PATH)$(DATA_PATH)/conf
	@mkdir -p $(BUILD_SRC_PATH)$(DATA_PATH)/data
	@mkdir -p $(BUILD_SRC_PATH)$(DATA_PATH)/logs
	@mkdir -p $(BUILD_SRC_PATH)$(DATA_PATH)/lookup
	@mkdir -p $(BUILD_SRC_PATH)$(DATA_PATH)/sql
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
	
	@echo -e "\tCopying over java runtime [error logs: $(BUILD_PATH)/logs/*.log]..."
	@cp -r $(JDK_PATH) $(BUILD_SRC_PATH)$(DATA_PATH) &>> $(BUILD_PATH)/logs/$(NAME)_copy.log

	@echo -e "\tCopying over tomcat binaries [error logs: $(BUILD_PATH)/logs/*.log]..."
	@cp -r $(TOMCAT_PATH) $(BUILD_SRC_PATH)$(DATA_PATH) &>> $(BUILD_PATH)/logs/$(NAME)_copy.log

	@echo -e "\tCopying over tissue stack start/conf scripts [error logs: $(BUILD_PATH)/logs/*.log]..."
	@cp -r $(PACKAGING_CONFIG_PATH)/$(NAME)-httpd.conf $(BUILD_SRC_PATH)$(DATA_PATH)/conf/$(NAME).conf &>> $(BUILD_PATH)/logs/$(NAME)_copy.log
	@if [ $(IS_DEBIAN_BASED) -gt 0 ] || [ $(IS_UBUNTU_BASED) -gt 0 ]; then \
		cp -r $(PACKAGING_CONFIG_PATH)/$(NAME).sh $(BUILD_SRC_PATH)$(BINS_PATH)/$(NAME) &>> $(BUILD_PATH)/logs/$(NAME)_copy.log; \
	else \
		cp -r $(PACKAGING_CONFIG_PATH)/$(NAME)_rpm.sh $(BUILD_SRC_PATH)$(BINS_PATH)/$(NAME) &>> $(BUILD_PATH)/logs/$(NAME)_copy.log; \
	fi
	@chmod 755 $(BUILD_SRC_PATH)$(BINS_PATH)/$(NAME)

	@echo -e "\tBuilding Tissue Stack Web Service (maven build). This may take a while, please be patient! [error logs: $(BUILD_PATH)/logs/*.log]..."
#	@cd $(BASE_DIR); git checkout $(CURRENT_BRANCH) &>$(BUILD_PATH)/logs/$(NAME)_copy.log || (echo "ERROR: git error => check logs !!!" && exit -1)
	@cd $(BASE_DIR);	mvn clean install &> $(BUILD_PATH)/logs/$(NAME)_maven_build.log

	@echo -e "\tCopying over war file into tomcat web application directory..."
	@cp $(BASE_DIR)target/TissueStack.war $(BUILD_SRC_PATH)$(DATA_PATH)/apache-tomcat-$(TOMCAT_VERSION)/webapps

	@echo -e "\tCreating sql update script containing the desired paths..."
	@cp  $(BASE_DIR)src/sql/*.sql $(BUILD_SRC_PATH)$(DATA_PATH)/sql
	@ echo -e "--THESE HAVE BEEN GENERATED AS PART OF THE MAKE PROCESS\n\
	UPDATE configuration SET value='$(DATA_PATH)/colormaps' WHERE name='colormaps_directory';\n\
	UPDATE configuration SET value='$(DATA_PATH)/tiles' WHERE name='server_tile_directory';\n\
	UPDATE configuration SET value='$(DATA_PATH)/upload' WHERE name='upload_directory';\n\
	UPDATE configuration SET value='$(DATA_PATH)/lookup' WHERE name='lookup_tables_directory';\n\
	UPDATE configuration SET value='$(DATA_PATH)/tasks/general' WHERE name='tasks_queue_file';\n\
	UPDATE configuration SET value='$(DATA_PATH)/data' WHERE name='data_directory';\n\
	UPDATE configuration SET value='$(DATA_PATH)/ands/datasets.xml' WHERE name='ands_dataset_xml';\n\
	" > $(BUILD_SRC_PATH)$(DATA_PATH)/sql/update_$(NAME)_config.sql;

	@echo -e "\tCreating environment script containing the desired paths..."
	@ echo -e "#!/bin/bash\n\
	TISSUE_STACK_HOME=$(DATA_PATH)\n\
	TISSUE_STACK_JNI_LIB_PATH=$(JNI_LIB_PATH)\n\
	IMAGE_SERVER_EXE=$(BINS_PATH)/$(EXE_NAME)\n\
	JAVA_HOME=\$$TISSUE_STACK_HOME/jdk1.6.0_25\n\
	CATALINA_HOME=\$$TISSUE_STACK_HOME/apache-tomcat-7.0.35\n\
	PATH=\$$PATH:$(BINS_PATH)\n\
	export TISSUE_STACK_HOME TISSUE_STACK_JNI_LIB_PATH IMAGE_SERVER_EXE JAVA_HOME CATALINA_HOME PATH" > $(BUILD_SRC_PATH)$(PROFILE_PATH)/$(NAME)_env.sh;
	@sudo chmod 755 $(BUILD_SRC_PATH)$(PROFILE_PATH)/$(NAME)_env.sh

	@echo -ne "\tCreating post install script for "
	@if [ $(IS_DEBIAN_BASED) -gt 0 ] || [ $(IS_UBUNTU_BASED) -gt 0 ]; then \
		echo -e "a debian based distribution..."; \
		echo -e "#!/bin/bash\n\
	rm -f /tmp/post-install.log\n\
	touch /tmp/post-install.log\n\
	chmod 666 /tmp/post-install.log\n\
	for dirs in \`find $(DATA_PATH)/ -name \"bin\"\`;do\n\
	\tchmod -R 755 \$$dirs/* &>> /tmp/post-install.log\n\
	done\n\
	service postgresql start &>> /tmp/post-install.log\n\
	sleep 5s\n\
	sudo su - postgres <<EOF\n\
	psql -U postgres -h localhost -f $(DATA_PATH)/sql/create_$(NAME)_db.sql &>> /tmp/post-install.log\n\
	psql -U postgres -h localhost -f $(DATA_PATH)/sql/create_$(NAME)_tables.sql tissuestack &>> /tmp/post-install.log\n\
	psql -U postgres -h localhost -f $(DATA_PATH)/sql/create_$(NAME)_config.sql tissuestack &>> /tmp/post-install.log\n\
	psql -U postgres -h localhost -f $(DATA_PATH)/sql/update_$(NAME)_config.sql tissuestack &>> /tmp/post-install.log\n\
	EOF\n\
	echo \"$(DATA_PATH)\" > /tmp/escaped.string\n\
	sed -i 's/\//\\\\\\\\\//g' /tmp/escaped.string\n\
	ESCAPED_STRING=\`cat /tmp/escaped.string\`\n\
	sed -i \"s/%%DOC_ROOT%%/\$$ESCAPED_STRING\\/web/g\" $(DATA_PATH)/conf/$(NAME).conf\n\
	sed -i 's/%%ERROR_LOG%%/\\/var\\/log\\/apache2\\/$(NAME)-error.log/g' $(DATA_PATH)/conf/$(NAME).conf\n\
	cp $(DATA_PATH)/conf/$(NAME).conf $(APACHE2_CONF_DIR)\n\
	a2ensite $(APACHE2_CONF_DIR)/$(NAME).conf\n\
	$(SYS_V_INIT_PATH)apache2 restart\
	" > $(BUILD_SRC_PATH)/post-install.sh; \
	else \
		echo -e "a red hat based distribution..."; \
	echo -e "#!/bin/bash\n\
	rm -f /tmp/post-install.log\n\
	touch /tmp/post-install.log\n\
	chmod 666 /tmp/post-install.log\n\
	for dirs in \`find $(DATA_PATH)/ -name \"bin\"\`;do\n\
	\tchmod -R 755 \$$dirs/* &>> /tmp/post-install.log\n\
	done\n\
	su -c \"su - postgres <<EOF\n\
	initdb &>> /tmp/post-install.log\n\
	EOF\n\
	chkconfig postgresql on &>> /tmp/post-install.log\n\
	service postgresql start &>> /tmp/post-install.log\n\
	sleep 5s\n\
	su -c \"su - postgres <<EOF\n\
	psql -U postgres -h localhost -f $(DATA_PATH)/sql/create_$(NAME)_db.sql &>> /tmp/post-install.log\n\
	psql -U postgres -h localhost -f $(DATA_PATH)/sql/create_$(NAME)_tables.sql tissuestack &>> /tmp/post-install.log\n\
	psql -U postgres -h localhost -f $(DATA_PATH)/sql/create_$(NAME)_config.sql tissuestack &>> /tmp/post-install.log\n\
	psql -U postgres -h localhost -f $(DATA_PATH)/sql/update_$(NAME)_config.sql tissuestack &>> /tmp/post-install.log\n\
	EOF\n\"\
	echo \"$(DATA_PATH)\" > /tmp/escaped.string\n\
	sed -i 's/\//\\\\\\\\\//g' /tmp/escaped.string\n\
	ESCAPED_STRING=\`cat /tmp/escaped.string\`\n\
	sed -i \"s/%%DOC_ROOT%%/\$$ESCAPED_STRING\\/web/g\" $(DATA_PATH)/conf/$(NAME).conf\n\
	sed -i 's/%%ERROR_LOG%%/\\/var\\/log\\/apache2\\/$(NAME)-error.log/g' $(DATA_PATH)/conf/$(NAME).conf\n\
	cp $(DATA_PATH)/conf/$(NAME).conf $(HTTPD_CONF_DIR)\n\
	mv $(HTTPD_CONF_DIR)/welcome.conf $(HTTPD_CONF_DIR)/welcome.conf.disabled\n\
	$(SYS_V_INIT_PATH)httpd restart\
	" > $(BUILD_SRC_PATH)/post-install.sh; \
	fi
	@sudo chmod 755 $(BUILD_SRC_PATH)/post-install.sh

	@echo -e "\tZipping up everything [log: $(BUILD_PATH)/logs/$(NAME)_tar.log ]..."
	@cd $(BUILD_SRC_PATH); tar cvzf $(BUILD_PATH)/$(DIST_NAME) * &> $(BUILD_PATH)/logs/$(NAME)_tar.log
	
	@echo -e "\n\tFinished creating binary archive:  $(BUILD_PATH)/$(DIST_NAME)"
		
.PHONY: all prepare compile install clean re recompile
