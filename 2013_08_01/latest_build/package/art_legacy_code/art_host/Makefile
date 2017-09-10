################################################################
#
#	Makefile for art_linux
#
#	Author : Daniel Wang
#
#	Version 1.0 @ July 1st 2011
#
################################################################

CODEDIR := $(CURDIR)
SUBDIRS := art_rel
export CODEDIR
export SUBDIRS

export TOOL_PREFIX=$(CROSS_COMPILE)

all: 
	@echo make all for $(SUBDIRS)
	for dir in $(SUBDIRS); do $(MAKE) -C $$dir MODULE=$$dir; done

depend: version Makefile
	@echo make depend in $(SUBDIRS)
	for dir in $(SUBDIRS); do $(MAKE) -C $$dir depend MODULE=$$dir; done

clean: 
	@echo make clean in $(SUBDIRS)  
	for dir in $(SUBDIRS); do $(MAKE) -C $$dir clean MODULE=$$dir; done

.PHONY : all version clean depend
