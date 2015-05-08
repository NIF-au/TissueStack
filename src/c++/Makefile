NAME			=	TissueStack Server

SRCS_COMMON		=	$(wildcard common/*.cpp)
SRCS_NETWORKING	=	$(wildcard networking/*.cpp)
SRCS_IMAGING	=	$(wildcard imaging/*.cpp)
SRCS_EXECUTION	=	$(wildcard execution/*.cpp)
SRCS_DATABASE	=	$(wildcard database/*.cpp)
SRCS_SERVICES	=	$(wildcard services/*.cpp)
SRCS_UTILS		=	$(wildcard utils/*.cpp)
SRCS			=	$(wildcard *.cpp)

INCLUDE			=	-Iinclude -I/usr/include/nifti \
					-Icommon/include -Iexecution/include \
					-Idatabase/include -Iservices/include \
					-Inetworking/include -Iimaging/include \
					-Iutils/include

LIB_PATH		=	-L/usr/lib64/dcmtk

IS_CENTOS_OR_FEDORA	?= 0
IS_RELEASE		?= 0

ifeq ($(IS_CENTOS_OR_FEDORA), 0)
LIBS			=	-lrt -lpthread -ldl -lpqxx -lpq -lcrypto -luuid \
					-lminc2 -lniftiio -ldcmdata -ldcmimgle -ldcmimage \
					-ldcmjpls -ldcmjpeg -lijg8 -lijg12 -lijg16 \
					-loflog -lofstd -lm -lz -lznz -lzip
else
LIBS			=	-lrt -lpthread -ldl -lpqxx -lpq -lcrypto -luuid \
					-lminc2 -lniftiio -ldcmdata -ldcmimgle -ldcmimage \
					-ldcmjpls -ldcmjpeg -lijg8 -lijg12 -lijg16 -lpng \
					-loflog -lofstd -lm -lz -lznz -lzip -lCharLS -ltiff
endif

EXE_NAME		=	TissueStackServer

ifeq ($(IS_RELEASE), 0)
FLAGS			=	-Wall -Werror -ggdb -std=c++11 -std=gnu++11 -std=c++0x
else
FLAGS			=	-Wall -Werror -std=c++11 -std=gnu++11 -std=c++0x
endif

CC				=	g++

OBJS_COMMON		=	$(SRCS_COMMON:%.cpp=%.o)
OBJS_NETWORKING	=	$(SRCS_NETWORKING:%.cpp=%.o)
OBJS_IMAGING	=	$(SRCS_IMAGING:%.cpp=%.o)
OBJS_EXECUTION	=	$(SRCS_EXECUTION:%.cpp=%.o)
OBJS_DATABASE	=	$(SRCS_DATABASE:%.cpp=%.o)
OBJS_SERVICES	=	$(SRCS_SERVICES:%.cpp=%.o)
OBJS_UTILS		=	$(SRCS_UTILS:%.cpp=%.o)
OBJS			=	$(SRCS:%.cpp=%.o)

# important configuration variables with defaults
export APP_NAME		=	tissuestack
export VERSION	=	2.0
export APPLICATION_PATH ?= /usr/local/$(APP_NAME)
export DATA_PATH ?= /opt/$(APP_NAME)
# the application path including the version
export APPLICATION_ROOT	=	$(APPLICATION_PATH)/$(VERSION)
# the destination for the binaries
export BINS_PATH			=	$(APPLICATION_ROOT)/bin
# packaging/install configuration
export IS_DEBIAN_BASED=`cat /etc/*-release | grep -i debian | wc -c`
export IS_UBUNTU_BASED=`cat /etc/*-release | grep -i ubuntu | wc -c`
export IS_CENTOS_BASED=`cat /etc/*-release | grep -i centos | wc -c`

#build path for binary distribution
export BASE_DIR			= 	./../../
export PACKAGING_CONFIG_PATH=	$(BASE_DIR)packaging
export PROFILE_PATH		= 	/etc/profile.d
export BUILD_PATH		=	/tmp/$(APP_NAME)_build
export BUILD_SRC_PATH		=	$(BUILD_PATH)/src
export DIST_NAME		=	$(APP_NAME)-$(VERSION).tar.gz
export SRC_DIST_NAME		=	$(APP_NAME)-$(VERSION)-sources.tar.gz

all:	clean prepare compile install compile-tools install-tools

%.o: %.cpp
	@echo -e "\tCompiling \"$(NAME)\" => [$(@)]"
	@$(CC) -DAPPLICATION_PATH='"$(DATA_PATH)"' -c $(LIBS) $(FLAGS) $(INCLUDE) `GraphicsMagick-config --cppflags --libs --ldflags` -o $@ -fPIC -pie $<
	
prepare:
	@echo -e "\n\t###################### TISSUE STACK BUILD [Version: $(VERSION)] ###################"
	@echo -e "\t    APPLICATION DATA:      $(DATA_PATH)"
	@echo -e "\t    BINARIES PATH:         $(BINS_PATH)"
	@echo -e "\t###################### TISSUE STACK BUILD [Version: $(VERSION)] ###################\n"

compile:	$(OBJS_COMMON) $(OBJS_NETWORKING) $(OBJS_DATABASE) $(OBJS_IMAGING) \
			$(OBJS_EXECUTION)  $(OBJS_SERVICES) $(OBJS_UTILS) $(OBJS)
	@echo -e "\tCompiling \"$(NAME)\" => $(EXE_NAME)"
	@$(CC) -DAPPLICATION_PATH='"$(DATA_PATH)"' $(OBJS_COMMON) $(OBJS_NETWORKING) $(OBJS_EXECUTION) $(OBJS_DATABASE) \
							$(OBJS_SERVICES) $(OBJS_IMAGING) $(OBJS_UTILS) $(OBJS) \
							`GraphicsMagick-config --cppflags --libs --ldflags` -o $(EXE_NAME) \
							$(LIB_PATH) $(LIBS) $(FLAGS) $(INCLUDE)

compile-tools:
	@make --no-print-directory -C tools/ compile

install-tools:
	@make --no-print-directory -C tools/ install
	
install: compile
	@echo -e "\n\tInstalling $(NAME) (requires super user priviledges):"
	@echo -e "\t-----------------------------------------------\n"
	@echo -e "\tInstalling '$(EXE_NAME)' executable into: $(BINS_PATH)."
	@sudo rm -rf $(BINS_PATH)
	@sudo mkdir -p $(BINS_PATH)
	@if [ ! -f $(EXE_NAME) ]; then echo "\nExecutable '$(EXE_NAME)' does not exist!"; fi;
	@sudo cp $(EXE_NAME) $(BINS_PATH)/$(EXE_NAME)
	@echo -e "\n\tFinished installation of $(NAME).\n"  
        
clean:
	@rm -rf *.o *.so *~ core $(EXE_NAME)
	@rm -rf common/*.o common/*.so common/core
	@rm -rf utils/*.o utils/*.so utils/*~ utils/core
	@rm -rf database/*.o database/*.so database/*~ utils/core
	@rm -rf services/*.o services/*.so services/*~ services/core
	@rm -rf networking/*.o networking/*.so networking/*~ networking/core
	@rm -rf imaging/*.o imaging/*.so imaging/*~ imaging/core
	@rm -rf execution/*.o execution/*.so execution/*~ utils/core
	@make --no-print-directory -C tools/ clean > /dev/null
	@echo -e "\n\tCleaned \"$(NAME)\"\n"
	
dist: all
	@echo -e "\n\n\tBuilding binary archive: $(DIST_NAME):"
	@echo -e "\t---------------------------------------------------"
	
	@echo -e "\n\tPreparing build directory..."
	@rm -rf $(BUILD_PATH)
	@mkdir -p $(BUILD_PATH)/logs
	@mkdir -p $(BUILD_SRC_PATH)$(BINS_PATH)
	@mkdir -p $(BUILD_SRC_PATH)$(PROFILE_PATH)
	@mkdir -p $(BUILD_SRC_PATH)$(DATA_PATH)/ands
	@mkdir -p $(BUILD_SRC_PATH)$(DATA_PATH)/conf
	@mkdir -p $(BUILD_SRC_PATH)$(DATA_PATH)/colormaps
	@mkdir -p $(BUILD_SRC_PATH)$(DATA_PATH)/sql

	@echo -e "\tCopying over Tissue Stack Image Server & Tools executables"
	@cp $(BINS_PATH)/* $(BUILD_SRC_PATH)$(BINS_PATH)
	
	@echo -e "\tCopying over front-end resources and colormaps..."
	@cp $(BASE_DIR)data/colormaps/* $(BUILD_SRC_PATH)$(DATA_PATH)/colormaps
	@cp -r $(BASE_DIR)/src/web $(BUILD_SRC_PATH)$(DATA_PATH)
	@ln -s $(DATA_PATH)/tiles $(BUILD_SRC_PATH)$(DATA_PATH)/web/tiles
	
	@echo -e "\tCopying over tissue stack start/conf scripts [error logs: $(BUILD_PATH)/logs/*.log]..."
	@cp -f $(PACKAGING_CONFIG_PATH)/$(APP_NAME)-httpd.conf $(BUILD_SRC_PATH)$(DATA_PATH)/conf/$(APP_NAME).conf &>> $(BUILD_PATH)/logs/$(APP_NAME)_copy.log
	@cp -f $(PACKAGING_CONFIG_PATH)/$(APP_NAME).sh $(BUILD_SRC_PATH)$(BINS_PATH)/$(APP_NAME) &>> $(BUILD_PATH)/logs/$(APP_NAME)_copy.log
	@if [ $(IS_DEBIAN_BASED) -gt 0 ] || [ $(IS_UBUNTU_BASED) -gt 0 ]; then \
		cp -f $(PACKAGING_CONFIG_PATH)/$(APP_NAME)_deb_init.sh $(BUILD_SRC_PATH)$(DATA_PATH)/conf/$(APP_NAME)_init.sh &>> $(BUILD_PATH)/logs/$(APP_NAME)_copy.log; \
	else \
		cp -f $(PACKAGING_CONFIG_PATH)/$(APP_NAME)_init.sh $(BUILD_SRC_PATH)$(DATA_PATH)/conf/$(APP_NAME)_init.sh &>> $(BUILD_PATH)/logs/$(APP_NAME)_copy.log; \
	fi
	@chmod 755 $(BUILD_SRC_PATH)$(BINS_PATH)/$(APP_NAME)

	@echo -e "\tCreating sql update script containing the desired paths..."
	@cp  $(BASE_DIR)src/sql/*.sql $(BUILD_SRC_PATH)$(DATA_PATH)/sql
	@ echo -e "--THESE HAVE BEEN GENERATED AS PART OF THE MAKE PROCESS\n\
	UPDATE configuration SET value='$(DATA_PATH)/colormaps' WHERE name='colormaps_directory';\n\
	UPDATE configuration SET value='$(DATA_PATH)/tiles' WHERE name='server_tile_directory';\n\
	UPDATE configuration SET value='$(DATA_PATH)/upload' WHERE name='upload_directory';\n\
	UPDATE configuration SET value='$(DATA_PATH)/lookup' WHERE name='lookup_directory';\n\
	UPDATE configuration SET value='$(DATA_PATH)/tasks' WHERE name='tasks_directory';\n\
	UPDATE configuration SET value='$(DATA_PATH)/data' WHERE name='data_directory';\n\
	UPDATE configuration SET value='$(DATA_PATH)/ands/datasets.xml' WHERE name='ands_dataset_xml';\n\
	" > $(BUILD_SRC_PATH)$(DATA_PATH)/sql/update_$(APP_NAME)_config.sql;

	@echo -e "\tCreating environment script containing the desired paths..."
		@if [ $(IS_DEBIAN_BASED) -gt 0 ] && [ $(IS_UBUNTU_BASED) -eq 0 ]; then \
		 echo "#!/bin/bash\n\
TISSUE_STACK_HOME=$(DATA_PATH)\n\
IMAGE_SERVER_EXE=$(BINS_PATH)/$(EXE_NAME)\n\
PATH=\$$PATH:$(BINS_PATH)\n\
HDF5_DISABLE_VERSION_CHECK=1\n\
export HDF5_DISABLE_VERSION_CHECK TISSUE_STACK_HOME IMAGE_SERVER_EXE PATH" > $(BUILD_SRC_PATH)$(PROFILE_PATH)/$(APP_NAME)_env.sh; \
        else \
                 echo -e "#!/bin/bash\n\
TISSUE_STACK_HOME=$(DATA_PATH)\n\
IMAGE_SERVER_EXE=$(BINS_PATH)/$(EXE_NAME)\n\
PATH=\$$PATH:$(BINS_PATH)\n\
HDF5_DISABLE_VERSION_CHECK=1\n\
export HDF5_DISABLE_VERSION_CHECK TISSUE_STACK_HOME IMAGE_SERVER_EXE PATH" > $(BUILD_SRC_PATH)$(PROFILE_PATH)/$(APP_NAME)_env.sh; \
        fi
	@if [ $(IS_CENTOS_BASED) -gt 0 ]; then \
		echo "export DCMDICTPATH=/usr/share/dcmtk/dicom.dic" >> $(BUILD_SRC_PATH)$(PROFILE_PATH)/$(APP_NAME)_env.sh; \
	fi

	@chmod 755 $(BUILD_SRC_PATH)$(PROFILE_PATH)/$(APP_NAME)_env.sh

	@if [ $(IS_DEBIAN_BASED) -gt 0 ] || [ $(IS_UBUNTU_BASED) -gt 0 ]; then \
		mkdir -p $(BUILD_SRC_PATH)$(DATA_PATH)/conf; \
		echo -e "\tCopying over pre,post and uninstall scripts for packaging ..."; \
		cp -f $(PACKAGING_CONFIG_PATH)/pre-install.sh $(BUILD_SRC_PATH)/pre-install.sh; \
		chmod 755 $(BUILD_SRC_PATH)/pre-install.sh; \
		cp -f $(PACKAGING_CONFIG_PATH)/post-install.sh $(BUILD_SRC_PATH)/post-install.sh; \
		chmod 755 $(BUILD_SRC_PATH)/post-install.sh; \
		cp -f $(PACKAGING_CONFIG_PATH)/pre-uninstall.sh $(BUILD_SRC_PATH)/pre-uninstall.sh; \
		chmod 755 $(BUILD_SRC_PATH)/pre-uninstall.sh; \
		cp -f $(PACKAGING_CONFIG_PATH)/post-uninstall.sh $(BUILD_SRC_PATH)/post-uninstall.sh; \
		chmod 755 $(BUILD_SRC_PATH)/post-uninstall.sh; \
	fi

	@echo -e "\tZipping up everything [log: $(BUILD_PATH)/logs/$(APP_NAME)_tar.log ]..."
	@cd $(BUILD_SRC_PATH); tar cvzf $(BUILD_PATH)/$(DIST_NAME) * &> $(BUILD_PATH)/logs/$(APP_NAME)_tar.log
	
	@echo -e "\n\tFinished creating binary archive:  $(BUILD_PATH)/$(DIST_NAME)"
