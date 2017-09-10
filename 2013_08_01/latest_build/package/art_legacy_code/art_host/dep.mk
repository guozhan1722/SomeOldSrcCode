depend: depend.mk
depend.mk: ../Makefile ../rules.mk ../dep.mk Makefile $(SRC)
		rm -f $@
ifdef SRC
		for f in $(SRC) ; do \
			$(CC) -M $(CFLAGS) $$f | sed -e 's,.*o[ :], &,' >> $@ ; \
		done
endif

include depend.mk
