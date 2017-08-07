APP=./webcam
LIBS=-lopencv_core -lopencv_contrib -lopencv_video -lopencv_highgui

SOURCES=main.cpp

INCLUDE=-I/usr/local/include/
LDFLAGS=-L/usr/local/lib/

.PHONY: default clean

default: $(APP)

$(APP): $(SOURCES)
	g++ -o $@ $(SOURCES) $(LDFLAGS) $(LIBS) $(INCLUDE)

clean:
	rm -f *.o $(APP)
