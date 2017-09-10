#GCC = arm-openwrt-linux-uclibc-g++
GCC=mips-openwrt-linux-uclibc-g++

CXXFLAGS=-O2 -L ../GobiConnectionMgmt/ -I../GobiConnectionMgmt/ -I../Shared -I../Core/ -fPIC

LDFLAGS=-lGobiConnectionMgmt -lstdc++ -lpthread -ldl

DATA_OBJS=gobi-data.o common.o
GPS_OBJS=gobi-gps.o common.o

all: gobi-data gobi-gps

o.cpp: 
	${GCC} ${CXXFLAGS} -c $<

gobi-data: ${DATA_OBJS}
	${GCC} ${CXXFLAGS} ${DATA_OBJS} -o $@ ${LDFLAGS}

gobi-gps: ${GPS_OBJS}
	${GCC} -O2 ${CXXFLAGS} ${GPS_OBJS} -o $@  ${LDFLAGS}

clean:
	rm -rf gobi-gps gobi-data *.o



