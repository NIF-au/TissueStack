NAME			=	TissueStack Imaging

SRCS_COMMON		=	$(wildcard ../common/*.cpp)
SRCS_NETWORKING	=	$(wildcard ../networking/*.cpp)
SRCS_EXECUTION	=	$(wildcard ../execution/*.cpp)
SRCS_DATABASE	=	$(wildcard ../database/*.cpp)
SRCS_SERVICES	=	$(wildcard ../services/*.cpp)
SRCS_UTILS		=	$(wildcard ../utils/*.cpp)
SRCS			=	$(wildcard *.cpp)

INCLUDE			=	-Iinclude \
					-I../common/include -I../execution/include \
					-I../database/include -I../services/include \
					-I../networking/include -I../utils/include

LIB_PATH		=

LIBS			=	-lrt -ldl -lpqxx -lpq -lcrypto -luuid -lminc2 -lhdf5

LIB_NAME		=	libTissueStackImaging.so

INSTALL_DIR		=	/tmp

FLAGS			=	-Wall -Werror -ggdb -std=c++11 -std=gnu++11 -std=c++0x 

CC				=	g++

OBJS_UTILS		=	$(SRCS_UTILS:%.cpp=%.o)
OBJS_COMMON		=	$(SRCS_COMMON:%.cpp=%.o)
OBJS_EXECUTION	=	$(SRCS_EXECUTION:%.cpp=%.o)
OBJS_NETWORKING	=	$(SRCS_NETWORKING:%.cpp=%.o)
OBJS_DATABASE	=	$(SRCS_DATABASE:%.cpp=%.o)
OBJS_SERVICES	=	$(SRCS_SERVICES:%.cpp=%.o)
OBJS			=	$(SRCS:%.cpp=%.o)

%.o: %.cpp
	@echo -e "\tCompiling \"$(NAME)\" => [$(@)]"
	@$(CC) -c $(LIBS) $(FLAGS) $(INCLUDE) `GraphicsMagick-config --cppflags --libs --ldflags` -o $@ -fPIC -pie $<
	
all:	compile install

compile: $(OBJS_UTILS) $(OBJS_COMMON) $(OBJS_EXECUTION) $(OBJS_NETWORKING) $(OBJS)
	@$(CC) $(OBJS) $(OBJS_COMMON)  $(OBJS_UTILS) $(OBJS_EXECUTION) -o $(LIB_NAME) $(INCLUDE) $(LIB_PATH) $(LIBS) $(FLAGS) -shared

install:
	@if [ -f $(LIB_NAME) ]; then mv $(LIB_NAME) $(INSTALL_DIR); fi;
	@echo -e "\n\tInstalled \"$(LIB_NAME)\" into \"$(INSTALL_DIR)\"\n"

test:	clean
	@echo -e "\tCompiling Test for \"$(NAME)\" => [test/test]"
	@$(CC) $(SRCS_COMMON) $(SRCS_DATABASE) $(SRCS_SERVICES) $(SRCS_UTILS) $(SRCS_EXECUTION) $(SRCS_NETWORKING) $(SRCS) test/test.cpp -o test/test `GraphicsMagick-config --cppflags --libs --ldflags` $(LIBS) $(FLAGS) $(INCLUDE)
	@echo -en "\tRunning Test for \"$(NAME)\" ... "
	@test/test 2&> test/output.log; if [ $$? -eq 0 ]; then echo " FAILED (check test/output.log)!"; else echo " SUCCESS.";fi; 
        
clean:
	@rm -rf *.o *.so *~ core $(LIB_NAME) test/test test/*.o test/*.log
	@echo -e "\n\tCleaned \"$(NAME)\"\n"
