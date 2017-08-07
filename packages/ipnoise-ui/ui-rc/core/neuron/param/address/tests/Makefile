LDFLAGS=-lsqlite3
APPNAME=test

.PHONY: default clean

default: $(APPNAME)

$(APPNAME):
	g++ -o $(APPNAME) $(LDFLAGS) main.cpp

clean:
	rm -f $(APPNAME)

