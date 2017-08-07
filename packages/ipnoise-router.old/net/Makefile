CFLAGS=$(INCLUDE) $(LIBDIR) -ggdb
LDFLAGS=
CC=g++

CTAGS=ctags

default: $(CTAGS) icmp.o
	
ctags:
	rm -f tags
	ctags -R .

icmp.o: icmp.cpp icmp.h
	$(CC) $(CFLAGS) -c -o $@ $(@:.o=.cpp)

clean:
	rm -f *.o *.a $(TARGET) tags 

