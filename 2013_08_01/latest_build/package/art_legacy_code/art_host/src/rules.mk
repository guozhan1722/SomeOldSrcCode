#ARTCOMPILESEL := HOSTCOMPILE
export ARTCOMPILESEL

ifeq ($(ARTCOMPILESEL),HOSTCOMPILE)
CC := gcc
AR := ar
AR_OPTS := -cvq
COMMONCFLAG := -c -fpack-struct=4 
else
AR_OPTS := -cvq
COMMONCFLAG := -c -fpack-struct=4 -DSWAPSOCKET
endif



SRC = $(SRC1) $(SRC2) $(SRC2)
OBJS = $(OBJS1) $(OBJS2) $(OBJS2)
