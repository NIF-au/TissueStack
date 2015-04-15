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

INCLUDE			=	-Iinclude -I/usr/include/nifti \
					-I../common/include -I../execution/include \
					-I../database/include -I../services/include \
					-I../networking/include -I../imaging/include \
					-I../utils/include

LIB_PATH		=	-L/usr/lib64/dcmtk

IS_CENTOS_OR_FEDORA	?= 0;

ifeq ($(IS_CENTOS_OR_FEDORA), 0)
LIBS			=	-lrt -lpthread -ldl -lpqxx -lpq -lcrypto -luuid \
					-lminc2 -lniftiio -ldcmdata -ldcmimgle -ldcmimage \
					-ldcmjpls -ldcmjpeg -lijg8 -lijg12 -lijg16 \
					-loflog -lofstd -lm -lz -lznz -lzip
else
LIBS			=	-lrt -lpthread -ldl -lpqxx -lpq -lcrypto -luuid \
					-lminc2 -lniftiio -ldcmdata -ldcmimgle -ldcmimage \
					-ldcmjpls -ldcmjpeg -lijg8 -lijg12 -lijg16 \
					-loflog -lofstd -lm -lz -lznz -lzip -lCharLS
endif

BINS_PATH		?= /tmp

TILER_EXE_NAME		=	TissueStackPreTiler
CONVERTER_EXE_NAME	=	TissueStackConverter

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

all: clean compile install

compile:	$(OBJS_COMMON) $(OBJS_NETWORKING) $(OBJS_DATABASE) $(OBJS_IMAGING) \
			$(OBJS_EXECUTION)  $(OBJS_SERVICES) $(OBJS_UTILS)
	@echo -e "\tCompiling \"$(NAME)\" => $(TILER_EXE_NAME)"
	@$(CC)   $(OBJS_COMMON) $(OBJS_NETWORKING) $(OBJS_EXECUTION) $(OBJS_DATABASE) \
							$(OBJS_SERVICES) $(OBJS_IMAGING) $(OBJS_UTILS) $(SRCS_TILER) \
							`GraphicsMagick-config --cppflags --libs --ldflags` -o $(TILER_EXE_NAME) \
							$(LIB_PATH) $(LIBS) $(FLAGS) $(INCLUDE)
	@echo -e "\tCompiling \"$(NAME)\" => $(CONVERTER_EXE_NAME)"
	@$(CC)   $(OBJS_COMMON) $(OBJS_NETWORKING) $(OBJS_EXECUTION) $(OBJS_DATABASE) \
							$(OBJS_SERVICES) $(OBJS_IMAGING) $(OBJS_UTILS) $(SRCS_CONVERTER) \
							`GraphicsMagick-config --cppflags --libs --ldflags` -o $(CONVERTER_EXE_NAME) \
							$(LIB_PATH) $(LIBS) $(FLAGS) $(INCLUDE)

install:
	@echo -e "\n\tInstalling $(NAME) (requires super user priviledges):"
	@echo -e "\t-----------------------------------------------\n"
	@sudo mkdir -p $(BINS_PATH)
	@echo -e "\tInstalling '$(TILER_EXE_NAME)' executable into: $(BINS_PATH)."
	@if [ ! -f $(TILER_EXE_NAME) ]; then echo "\nExecutable '$(TILER_EXE_NAME)' does not exist!"; fi;
	@sudo cp $(TILER_EXE_NAME) $(BINS_PATH)/$(TILER_EXE_NAME)
	@echo -e "\tInstalling '$(CONVERTER_EXE_NAME)' executable into: $(BINS_PATH)."
	@if [ ! -f $(CONVERTER_EXE_NAME) ]; then echo "\nExecutable '$(CONVERTER_EXE_NAME)' does not exist!"; fi;
	@sudo cp $(CONVERTER_EXE_NAME) $(BINS_PATH)/$(CONVERTER_EXE_NAME)
	@echo -e "\n\tFinished installation of $(NAME).\n"  

clean:
	@rm -rf *.o *.so *~ core $(CONVERTER_EXE_NAME) $(TILER_EXE_NAME)
	@rm -rf ../common/*.o ../common/*.so ../common/core
	@rm -rf ../utils/*.o ../utils/*.so utils/*~ utils/core
	@rm -rf ../database/*.o ../database/*.so ../database/*~ ../utils/core
	@rm -rf ../services/*.o ../services/*.so ../services/*~ ../services/core
	@rm -rf ../networking/*.o ../networking/*.so ../networking/*~ ../networking/core
	@rm -rf ../imaging/*.o ../imaging/*.so ../imaging/*~ ../imaging/core
	@rm -rf ../execution/*.o ../execution/*.so ../execution/*~ ../utils/core
	@echo -e "\n\tCleaned \"$(NAME)\"\n"
