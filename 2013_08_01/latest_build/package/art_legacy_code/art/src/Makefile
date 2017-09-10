-include .makefile

ifeq ($(NO_GPIB),1)
export CFLAGS+=-DNO_GPIB
endif

all: art

.PHONY: art

art clean:
	$(MAKE) -C dk/mdk/devlib $(MAKECMDGOALS)
	$(MAKE) -C dk/mdk/art $(MAKECMDGOALS)
