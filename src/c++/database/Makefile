NAME			=	TissueStack Database

SRCS_COMMON		=	$(wildcard ../common/*.cpp)
SRCS_EXECUTION	=	$(wildcard ../execution/*.cpp)
SRCS_NETWORKING	=	$(wildcard ../networking/*.cpp)
SRCS_IMAGING	=	$(wildcard ../imaging/*.cpp)
SRCS_UTILS		=	$(wildcard ../utils/*.cpp)
SRCS			=	$(wildcard *.cpp)

INCLUDE			=	-Iinclude -I../common/include -I../utils/include -I../execution/include -I../imaging/include -I../networking/include

LIB_PATH		=

LIBS			=	-lrt -lpthread -ldl -lpqxx -lpq 

LIB_NAME		=	libTissueStackDatabase.so

INSTALL_DIR		=	/tmp

FLAGS			=	-Wall -Werror -ggdb -std=c++11 -std=gnu++11 -std=c++0x 

CC				=	g++

OBJS_UTILS		=	$(SRCS_UTILS:%.cpp=%.o)
OBJS_COMON		=	$(SRCS_COMMON:%.cpp=%.o)
OBJS_IMAGING	=	$(SRCS_IMAGING:%.cpp=%.o)
OBJS_EXECUTION	=	$(SRCS_EXECUTION:%.cpp=%.o)
OBJS_NETWORKING	=	$(SRCS_NETWORKING:%.cpp=%.o)
OBJS			=	$(SRCS:%.cpp=%.o)

%.o: %.cpp
	@echo -e "\tCompiling \"$(NAME)\" => [$(@)]"
	@$(CC) -c $(LIBS) $(FLAGS) $(INCLUDE) `GraphicsMagick-config --cppflags --libs --ldflags` -o $@ -fPIC -pie $<
	
all:	compile install

compile: $(OBJS) $(OBJS_COMON) $(OBJS_UTILS) $(OBJS_EXECUTION) $(OBJS_IMAGING) $(OBJS_NETWORKING)
	@$(CC) $(OBJS) $(OBJS_COMON) $(OBJS_UTILS) $(OBJS_EXECUTION) $(OBJS_IMAGING) $(OBJS_NETWORKING) `GraphicsMagick-config --cppflags --libs --ldflags` -o $(LIB_NAME) $(INCLUDE) $(LIB_PATH) $(LIBS) $(FLAGS) -shared

install:
	@if [ -f $(LIB_NAME) ]; then mv $(LIB_NAME) $(INSTALL_DIR); fi;
	@echo -e "\n\tInstalled \"$(LIB_NAME)\" into \"$(INSTALL_DIR)\"\n"

test:	clean
	@echo -e "\tCompiling Test for \"$(NAME)\" => [test/test]"
	@$(CC) `GraphicsMagick-config --cppflags --libs --ldflags` -o test/test $(LIBS) $(FLAGS) $(INCLUDE) $(SRCS) $(SRCS_COMMON) $(SRCS_IMAGING) $(SRCS_NETWORKING) $(SRCS_EXECUTION) $(SRCS_UTILS) test/test.cpp 
	@echo -en "\tRunning Test for \"$(NAME)\" ... "
	@test/test 2&> test/output.log; if [ $$? -eq 0 ]; then echo " FAILED (check test/output.log)!"; else echo " SUCCESS.";fi; 
        
clean:
	@rm -rf *.o *.so *~ core $(LIB_NAME) test/test test/*.o test/*.log
	@echo -e "\n\tCleaned \"$(NAME)\"\n"
