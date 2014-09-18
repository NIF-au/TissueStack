NAME			=	TissueStack Tools

SRCS_COMMON		=	$(wildcard ../common/*.cpp)
SRCS_NETWORKING	=	$(wildcard ../networking/*.cpp)
SRCS_IMAGING	=	$(wildcard ../imaging/*.cpp)
SRCS_EXECUTION	=	$(wildcard ../execution/*.cpp)
SRCS_DATABASE	=	$(wildcard ../database/*.cpp)
SRCS_SERVICES	=	$(wildcard ../services/*.cpp)
SRCS_UTILS		=	$(wildcard ../utils/*.cpp)
SRCS_TILER		=	TissueStackPreTiler.cpp
SRCS_CONVERTER	=	TissueStackConverter.cpp

INCLUDE			=	-I../common/include -I../networking/include -I../execution/include \
					-I../imaging/include -I../utils/include -I../database/include -I../services/include

LIB_PATH		=

LIBS			=	-lrt -lpthread -ldl -lpqxx -lpq -lcrypto -luuid

TILER_EXE_NAME		=	TissueStackPreTiler
CONVERTER_EXE_NAME	=	TissueStackConverter

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
OBJS_TILER		=	$(SRCS_TILER:%.cpp=%.o)
OBJS_CONVERTER	=	$(SRCS_CONVERTER:%.cpp=%.o)

%.o: %.cpp
	@echo -e "\tCompiling \"$(NAME)\" => [$(@)]"
	@$(CC) -c $(LIBS) $(FLAGS) $(INCLUDE) `GraphicsMagick-config --cppflags --libs --ldflags` -o $@ -fPIC -pie $<

all:	compile install

compile:	$(OBJS_COMMON) $(OBJS_NETWORKING) $(OBJS_DATABASE) $(OBJS_IMAGING) \
			$(OBJS_EXECUTION)  $(OBJS_SERVICES) $(OBJS_UTILS) $(OBJS_TILER) $(OBJS_CONVERTER)
	@echo -e "\tCompiling \"$(NAME)\" => $(TILER_EXE_NAME)"
	@$(CC)   $(OBJS_COMMON) $(OBJS_NETWORKING) $(OBJS_EXECUTION) $(OBJS_DATABASE) \
							$(OBJS_SERVICES) $(OBJS_IMAGING) $(OBJS_UTILS) $(OBJS_TILER) \
							`GraphicsMagick-config --cppflags --libs --ldflags` -o $(TILER_EXE_NAME) \
							$(LIBS) $(FLAGS) $(INCLUDE)
	@echo -e "\tCompiling \"$(NAME)\" => $(CONVERTER_EXE_NAME)"
	@$(CC)   $(OBJS_COMMON) $(OBJS_NETWORKING) $(OBJS_EXECUTION) $(OBJS_DATABASE) \
							$(OBJS_SERVICES) $(OBJS_IMAGING) $(OBJS_UTILS) $(OBJS_CONVERTER) \
							`GraphicsMagick-config --cppflags --libs --ldflags` -o $(CONVERTER_EXE_NAME) \
							$(LIBS) $(FLAGS) $(INCLUDE)

install:
	@if [ -f $(TILER_EXE_NAME) ]; then mv $(TILER_EXE_NAME) $(INSTALL_DIR); fi;
	@echo -e "\n\tInstalled \"$(TILER_EXE_NAME)\" into \"$(INSTALL_DIR)\"\n"
	@if [ -f $(CONVERTER_EXE_NAME) ]; then mv $(CONVERTER_EXE_NAME) $(INSTALL_DIR); fi;
	@echo -e "\n\tInstalled \"$(CONVERTER_EXE_NAME)\" into \"$(INSTALL_DIR)\"\n"

clean:
	@rm -rf *.o *.so *~ core $(CONVERTER_EXE_NAME) $(TILER_EXE_NAME)
	@rm -rf ../common/*.o ../common/*.so ../common/core
	@make --no-print-directory -C ../utils/ clean > /dev/null
	@make --no-print-directory -C ../database/ clean > /dev/null
	@make --no-print-directory -C ../services/ clean > /dev/null
	@make --no-print-directory -C ../networking/ clean > /dev/null
	@make --no-print-directory -C ../imaging/ clean > /dev/null
	@make --no-print-directory -C ../execution/ clean > /dev/null
	@echo -e "\n\tCleaned \"$(NAME)\"\n"