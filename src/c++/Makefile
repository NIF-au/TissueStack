NAME			=	TissueStack Image Server

SRCS_COMMON		=	$(wildcard common/*.cpp)
SRCS_NETWORKING	=	$(wildcard networking/*.cpp)
SRCS_EXECUTION	=	$(wildcard execution/*.cpp)
SRCS_UTILS		=	$(wildcard utils/*.cpp)
SRCS			=	$(wildcard *.cpp)


INCLUDE			=	-Icommon/include -Inetworking/include -Iexecution/include -Iutils/include

LIB_PATH		=

LIBS			=	-lrt -lpthread -ldl

EXE_NAME		=	TissueStackImageServer

INSTALL_DIR		=	/tmp

FLAGS			=	-Wall -Werror -ggdb -std=c++11 -std=gnu++11 -std=c++0x 

CC				=	g++

OBJS_COMMON		=	$(SRCS_COMMON:%.cpp=%.o)
OBJS_NETWORKING	=	$(SRCS_NETWORKING:%.cpp=%.o)
OBJS_EXECUTION	=	$(SRCS_EXECUTION:%.cpp=%.o)
OBJS_UTILS		=	$(SRCS_UTILS:%.cpp=%.o)
OBJS			=	$(SRCS:%.cpp=%.o)
%.o: %.cpp
	@echo -e "\tCompiling \"$(NAME)\" => [$(@)]"
	@$(CC) -c $(LIBS) $(FLAGS) $(INCLUDE) -o $@ -fPIC -pie $<

all:	compile install


compile:	clean $(OBJS_COMMON) $(OBJS_NETWORKING) $(OBJS_EXECUTION) $(OBJS_UTILS) $(OBJS)
	@echo -e "\tCompiling \"$(NAME)\" => $(EXE_NAME)"
	@$(CC)   $(OBJS_COMMON) $(OBJS_NETWORKING) $(OBJS_EXECUTION) $(OBJS_UTILS) $(OBJS) -o $(EXE_NAME) $(LIBS) $(FLAGS) $(INCLUDE)

install:
	@if [ -f $(EXE_NAME) ]; then mv $(EXE_NAME) $(INSTALL_DIR); fi;
	@echo -e "\n\tInstalled \"$(EXE_NAME)\" into \"$(INSTALL_DIR)\"\n"
        
clean:
	@rm -rf *.o *.so *~ core $(EXE_NAME)
	@rm -rf common/*.o common/*.so common/core
	@make --no-print-directory -C utils/ clean > /dev/null
	@make --no-print-directory -C networking/ clean > /dev/null
	@make --no-print-directory -C execution/ clean > /dev/null
	@echo -e "\n\tCleaned \"$(NAME)\"\n"
