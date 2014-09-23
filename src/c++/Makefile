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

LIB_PATH		=

LIBS			=	-lrt -lpthread -ldl -lpqxx -lpq -lcrypto -luuid \
					-lminc2 -lhdf5 -lniftiio -lz -lznz -lm

EXE_NAME		=	TissueStackServer

INSTALL_DIR		=	/tmp

FLAGS			=	-Wall -Werror -ggdb -std=c++11 -std=gnu++11 -std=c++0x 

CC				=	g++

OBJS_COMMON		=	$(SRCS_COMMON:%.cpp=%.o)
OBJS_NETWORKING	=	$(SRCS_NETWORKING:%.cpp=%.o)
OBJS_IMAGING	=	$(SRCS_IMAGING:%.cpp=%.o)
OBJS_EXECUTION	=	$(SRCS_EXECUTION:%.cpp=%.o)
OBJS_DATABASE	=	$(SRCS_DATABASE:%.cpp=%.o)
OBJS_SERVICES	=	$(SRCS_SERVICES:%.cpp=%.o)
OBJS_UTILS		=	$(SRCS_UTILS:%.cpp=%.o)
OBJS			=	$(SRCS:%.cpp=%.o)
%.o: %.cpp
	@echo -e "\tCompiling \"$(NAME)\" => [$(@)]"
	@$(CC) -c $(LIBS) $(FLAGS) $(INCLUDE) `GraphicsMagick-config --cppflags --libs --ldflags` -o $@ -fPIC -pie $<

all:	compile install


compile:	$(OBJS_COMMON) $(OBJS_NETWORKING) $(OBJS_DATABASE) $(OBJS_IMAGING) \
			$(OBJS_EXECUTION)  $(OBJS_SERVICES) $(OBJS_UTILS) $(OBJS)
	@echo -e "\tCompiling \"$(NAME)\" => $(EXE_NAME)"
	@$(CC)   $(OBJS_COMMON) $(OBJS_NETWORKING) $(OBJS_EXECUTION) $(OBJS_DATABASE) \
							$(OBJS_SERVICES) $(OBJS_IMAGING) $(OBJS_UTILS) $(OBJS) \
							`GraphicsMagick-config --cppflags --libs --ldflags` -o $(EXE_NAME) \
							$(LIBS) $(FLAGS) $(INCLUDE)

install:
	@if [ -f $(EXE_NAME) ]; then mv $(EXE_NAME) $(INSTALL_DIR); fi;
	@echo -e "\n\tInstalled \"$(EXE_NAME)\" into \"$(INSTALL_DIR)\"\n"
        
clean:
	@rm -rf *.o *.so *~ core $(EXE_NAME)
	@rm -rf common/*.o common/*.so common/core
	@make --no-print-directory -C utils/ clean > /dev/null
	@make --no-print-directory -C database/ clean > /dev/null
	@make --no-print-directory -C services/ clean > /dev/null
	@make --no-print-directory -C networking/ clean > /dev/null
	@make --no-print-directory -C imaging/ clean > /dev/null
	@make --no-print-directory -C execution/ clean > /dev/null
	@echo -e "\n\tCleaned \"$(NAME)\"\n"
