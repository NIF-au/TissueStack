DIRSRC		=	./src/c

all: compile install

clean-tools:
	@make --no-print-directory -C $(DIRSRC) clean-tools
	
compile-tools:
	@make --no-print-directory -C $(DIRSRC) compile-tools

install-tools:
	@make --no-print-directory -C $(DIRSRC) install-tools
	
compile:	
	@make --no-print-directory -C $(DIRSRC) compile

install:
	@make --no-print-directory -C $(DIRSRC) install

clean:
	@make --no-print-directory -C $(DIRSRC) clean

dist:
	@make --no-print-directory -C $(DIRSRC) dist

source:
	@make --no-print-directory -C $(DIRSRC) source
	