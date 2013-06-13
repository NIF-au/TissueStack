all:	compile install

prepare:
# the version has to be set for a production build (not debug build)
ifndef VERSION
	$(error VERSION IS NOT SET !!!!!!!!!!!!! Please call make like this: make VERSION=X.X !!!!!)
endif

install: prepare
	@make --no-print-directory -C ./converter/ install
	@make --no-print-directory -C ./tiler/ install

clean:
	@make --no-print-directory -C ./converter/ clean
	@make --no-print-directory -C ./tiler/ clean

debug:	prepare
	@make --no-print-directory -C ./converter/ debug
	@make --no-print-directory -C ./tiler/ debug

compile: prepare
	@make --no-print-directory -C ./converter/ compile
	@make --no-print-directory -C ./tiler/ compile

recompile: clean compile

re: clean debug install

source: clean

.PHONY: all prepare compile install clean source re recompile
