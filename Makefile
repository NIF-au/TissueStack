DIRSRC		=	./src/c

all: compile install

compile:	
	make -C $(DIRSRC) compile

install:
	make -C $(DIRSRC) install

clean:
	make -C $(DIRSRC) clean
