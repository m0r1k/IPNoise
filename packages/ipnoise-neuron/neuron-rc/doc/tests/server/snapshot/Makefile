TARGET=main

.PHONY: default

default: $(TARGET)

$(TARGET):
	 g++ -o ./main main.cpp -I/home/morik/CImg-1.5.8_rolling140127/ -lX11 -lpng

clean:
	rm -f *.o
	rm -f $(TARGET)

