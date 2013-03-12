DIRSRC		=	./src/c

all: compile install

compile:	
	@make --no-print-directory -C $(DIRSRC) compile

install:
	@make --no-print-directory -C $(DIRSRC) install

clean:
	@make --no-print-directory -C $(DIRSRC) clean

dist:
	@make --no-print-directory -C $(DIRSRC) dist
	