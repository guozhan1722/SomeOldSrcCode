
SUBDIRS := 	GobiConnectionMgmt	\
		GobiImageMgmt		\
		ConnManager		\

all: 
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) )||exit 1 ; done             
 
                
clean:
	find -name *.so* | xargs rm -rf
	find -name *.o | xargs rm -rf
	find -name *.a | xargs rm -rf        


