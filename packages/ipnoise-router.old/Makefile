TARGET=ipnoise-router
INCLUDE_=$(INCLUDE) -I/usr/local/include -I/usr/include/libxml2/ -I../ -I./
CFLAGS=$(INCLUDE_) -L$(LIBDIR) -L/usr/local/lib/ -ggdb
LDFLAGS=-levent -lxml2
CC=g++
CTAGS=ctags

default: $(CTAGS) $(TARGET)

$(TARGET): main.o actions.o cookie.o parts.o chat.o uiSession.o	\
	netSession.o user.o base64.o routeClass.o neighbourClass.o
	cd hmac-md5 && make
	cd net									\
		&& INCLUDE="$(INCLUDE_)" 			\
		   LIBDIR="$(LIBDIR)" 				\
		   make
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ *.o 	\
		hmac-md5/cptBase16.o				\
		hmac-md5/cptBase64.o				\
		hmac-md5/cptHMAC.o					\
		hmac-md5/cptHash.o					\
		hmac-md5/cptMD5.o					\
		hmac-md5/gen-huid.o					\
		net/*.o								\
		$(LIBDIR)/*.a

$(CTAGS):
	rm -f tags;ctags -R .

actions.o: actions.cpp
	$(CC) $(CFLAGS) -c -o $@ $(@:.o=.cpp)

params.o: params.cpp params.h
	$(CC) $(CFLAGS) -c -o $@ $(@:.o=.cpp)

cookie.o: cookie.cpp cookie.h
	$(CC) $(CFLAGS) -c -o $@ $(@:.o=.cpp)

parts.o: parts.cpp parts.h
	$(CC) $(CFLAGS) -c -o $@ $(@:.o=.cpp)

chat.o: chat.cpp chat.h
	$(CC) $(CFLAGS) -c -o $@ $(@:.o=.cpp)

user.o: user.cpp user.h
	$(CC) $(CFLAGS) -c -o $@ $(@:.o=.cpp)

uiSession.o: uiSession.cpp uiSession.h
	$(CC) $(CFLAGS) -c -o $@ $(@:.o=.cpp)

netSession.o: netSession.cpp netSession.h
	$(CC) $(CFLAGS) -c -o $@ $(@:.o=.cpp)

base64.o: base64.cpp base64.h
	$(CC) $(CFLAGS) -c -o $@ $(@:.o=.cpp)

routeClass.o: routeClass.cpp routeClass.h
	$(CC) $(CFLAGS) -c -o $@ $(@:.o=.cpp)

neighbourClass.o: neighbourClass.cpp neighbourClass.h
	$(CC) $(CFLAGS) -c -o $@ $(@:.o=.cpp)

main.o: main.cpp
	$(CC) $(CFLAGS) -c -o $@ $(@:.o=.cpp)

clean:
	rm -f *.o $(TARGET) tags
	cd hmac-md5 && make clean
	cd net && make clean

