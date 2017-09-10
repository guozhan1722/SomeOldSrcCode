
#
# $Id: Makefile,v 1.144 2007/11/11 15:14:39 pkot Exp $
#
# Makefile for the GNOKII tool suite.
#
# Copyright (C) 1999 Hugh Blemings & Pavel Janík ml.
#               2000 Karel Zak
#

TOPDIR=.

#
# Makefile.global contains gnokii global settings
#
include ${TOPDIR}/Makefile.global

BIN_DIRS = gnokii

ifndef WIN32
BIN_DIRS += gnokiid utils
endif

DIRS += common \
	$(BIN_DIRS)

INSTALL_DIRS =	$(BIN_DIRS) \
		common

INSTALL_INCLUDES = include \
		   common

all: compile

compile: $(DIRS)
	@echo "done"

# build the apps after building the library   
$(BIN_DIRS): common

$(DIRS):
	$(MAKE) -C $@

clean:
	$(RM) *~ *.orig *.rej include/*~ include/*.orig include/*.rej testsuite/myout*
	@for dir in $(DIRS); do \
	    if [ -e $$dir/Makefile ]; then \
		$(MAKE) -C $$dir clean; \
	    fi; \
	done

	@echo "done"

distclean: clean
	$(MAKE) -C common distclean
	$(RM) Makefile.global config.cache config.log config.status \
		include/config.h \
		packaging/RedHat/gnokii.spec \
		packaging/Slackware/SlackBuild \
		packaging/Slackware/SlackBuild-xgnokii \
		po/Makefile.in
	$(RM) `$(FIND) . -name "*~"`
	@echo "done"

dep:
	@for dir in $(DIRS); do \
	    if [ -e $$dir/Makefile ]; then \
		$(MAKE) -C $$dir dep; \
	    fi; \
	done

	@echo "done"

test:
	( cd testsuite; ./testit )

install-binaries: compile
	@for dir in $(INSTALL_DIRS); do \
		if [ -e $$dir/Makefile ]; then \
			$(MAKE) -C $$dir install; \
		fi; \
	done

	@echo "done"

install-includes:
	@for dir in $(INSTALL_INCLUDES); do \
		if [ -e $$dir/Makefile ]; then \
			$(MAKE) -C $$dir install; \
		fi; \
	done

install: compile install-binaries

install-devel: compile install-binaries install-includes

.PHONY: all compile install clean distclean dep depend install-binaries $(DIRS)
