bin_PROGRAMS=websockserverbin 
libwebsockets_test_server_SOURCES=test-server.c
libwebsockets_test_server_LDADD=-L../lib -lwebsockets -lz -luci
 
if MINGW                                                                        
libwebsockets_test_server_CFLAGS:= -w  -I../win32port/win32helpers

libwebsockets_test_server_LDADD+= -lm -luser32 -ladvapi32 -lkernel32 -lgcc -lws2_32 -lz

else
libwebsockets_test_server_CFLAGS:= -Werror
endif   

libwebsockets_test_server_CFLAGS+= -Wall -std=gnu99 -pedantic -DINSTALL_DATADIR=\"@datadir@\" -DLWS_OPENSSL_CLIENT_CERTS=\"@clientcertdir@\"



#
# cook a random test cert and key
# notice your real cert and key will want to be 0600 permissions
libwebsockets-test-server.pem libwebsockets-test-server.key.pem:
	printf "GB\nErewhon\nAll around\nlibwebsockets-test\n\nlocalhost\nnone@invalid.org\n" | \
	openssl req -new -newkey rsa:1024 -days 10000 -nodes -x509 -keyout \
	./libwebsockets-test-server.key.pem -out ./libwebsockets-test-server.pem >/dev/null 2>&1  && \
	chmod 644 	./libwebsockets-test-server.key.pem \
			./libwebsockets-test-server.pem

clean-local:
	rm -f ./libwebsockets-test-server.key.pem ./libwebsockets-test-server.pem


