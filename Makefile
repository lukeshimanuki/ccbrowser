all: bin/ccbrowser

bin/ccbrowser: FORCE ccbrowser.c config.h defs.h
	gcc -Wall -g -o $@ ccbrowser.c -lcef

clean: FORCE
	rm -f bin/ccbrowser

FORCE:

