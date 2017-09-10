#
GCC = arm-openwrt-linux-uclibc-g++
CXXFLAGS+=-fPIC 
#GCC = g++
OBJ =  gobi.o\
      GobiApi.o \
      CallBackFun.o
      
all: clean gobi

#$(GCC) -rdynamic --static -o gobi gobi.cpp -ldl	

gobi: $(OBJ) 
	$(GCC) $(CXXFLAGS)  -rdynamic  $(OBJ) -o gobi -ldl -lstdc++ -lpthread
	
gobi.o: gobi.cpp
	$(GCC) -c gobi.cpp -o gobi.o
GobiApi.o:GobiApi.cpp 
	$(GCC) -c GobiApi.cpp -o GobiApi.o 
	
CallBackFun.o:CallBackFun.cpp
	$(GCC) -c CallBackFun.cpp -o CallBackFun.o
clean:
	-rm gobi $(OBJ)
