TARGET	= ./1
CFLAGS += -I/mnt/multimedia/ipnoise/IPNoise/packages/ -std=c++11 -pthread

.PHONY: default

default:
	g++ -o ${TARGET} ${CFLAGS}  test1.cpp

clean:
	rm -f *.o
	rm -f ${TARGET}

