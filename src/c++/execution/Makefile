NAME			=	TissueStack Execution

SRCS			=	$(wildcard *.cpp)
SRCS_COMMON		=	$(wildcard ../common/*.cpp)
SRCS_NETWORK	=	$(wildcard ../networking/*.cpp)
SRCS_UTILS		=	$(wildcard ../utils/*.cpp)

INCLUDE			=	-Iinclude -I../utils/include -I../common/include -I../networking/include

LIB_PATH		=

LIBS			=	-lrt -lpthread -ldl

LIB_NAME		=	libTissueStackThreading.so

INSTALL_DIR		=	/tmp

FLAGS			=	-Wall -Werror -ggdb -std=c++11 -std=gnu++11 -std=c++0x 

CC				=	g++

OBJS			=	$(SRCS:%.cpp=%.o)

%.o: %.cpp
	@echo -e "\tCompiling \"$(NAME)\" => [$(@)]"
	@$(CC) -c $(LIBS) $(FLAGS) $(INCLUDE) -o $@ -fPIC -pie $<
	
all:	compile install

compile: $(OBJS)
	@$(CC) $(OBJS) -o $(LIB_NAME) $(INCLUDE) $(LIB_PATH) $(LIBS) $(FLAGS) -shared

install:
	@if [ -f $(LIB_NAME) ]; then mv $(LIB_NAME) $(INSTALL_DIR); fi;
	@echo -e "\n\tInstalled \"$(LIB_NAME)\" into \"$(INSTALL_DIR)\"\n"

test:	clean
	@echo -e "\tCompiling test.so for \"$(NAME)\" => [/tmp/test.so]"
	@$(CC) -o /tmp/test.so test/test.so.cpp $(INCLUDE) $(LIB_PATH) $(LIBS) $(FLAGS) -fPIC -pie -shared
	@echo -e "\tCompiling Test for \"$(NAME)\" => [test/test]"
	@$(CC) -o test/test $(SRCS_NETWORK) $(SRCS_COMMON) $(SRCS_UTILS) $(SRCS) $(FLAGS) $(INCLUDE) $(LIBS) test/test.cpp
	@echo -en "\tRunning Test for \"$(NAME)\" ... "
	@test/test 2&> test/output.log; if [ $$? -eq 0 ]; then echo " FAILED (check test/output.log)!"; else echo " SUCCESS.";fi; 
	
clean:
	@rm -rf *.o *.so *~ core $(LIB_NAME) test/test test/*.o test/*.log
	@echo -e "\n\tCleaned \"$(NAME)\"\n"
