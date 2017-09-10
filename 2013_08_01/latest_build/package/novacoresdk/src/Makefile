MAIN_INCLUDE_DIR = .

SOURCES6=gps.cpp \
SdkWrapper.cpp \
GpsWrapper.cpp

OBJECTS6=$(SOURCES6:.cpp=.o)
EXECUTABLE6=gpscarrstart

#
#  The following linker command causes the executable to look for dynamically 
#  loadable shared objects in the current running directory.  Other options are
#  setting and exporting the LD_LIBRARY_PATH variable or copying the sdk modules to 
#  a build in path such as /usr/lib
#
# -Xlinker "-rpath=."
#
GCC=mips-openwrt-linux-uclibc-g++
CXXFLAGS=-O2 -I$(MAIN_INCLUDE_DIR) -I../headers -fPIC
#CC=g++
#CFLAGS=-c -Wall -D__linux__ -I$(MAIN_INCLUDE_DIR) -I../headers
#	$(GCC) $(OBJECTS) $(LDFLAGS) -o $@
LDFLAGS= -L. -lrt -lpthread -lstdc++ -ldl -Xlinker "-rpath=.:../bin"

all: $(EXECUTABLE6)
#all: $(EXECUTABLE1) $(EXECUTABLE2) $(EXECUTABLE3) $(EXECUTABLE4) $(EXECUTABLE5) \
#$(EXECUTABLE6) $(EXECUTABLE7) $(EXECUTABLE8) $(EXECUTABLE0)
	
$(EXECUTABLE6): $(OBJECTS6) 
	${GCC} ${CXXFLAGS} ${OBJECTS6} -o $@ ${LDFLAGS}
.cpp.o:
	${GCC} ${CXXFLAGS} -c $<
clean:
	rm $(EXECUTABLE6)
	rm *.o
