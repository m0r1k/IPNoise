TARGET=main

.PHONY: default clean

default: $(TARGET)

$(TARGET):
	gcc -o $@ *.c -lX11 -lGL -lGLU

clean:
	rm -f *.o
	rm -f $(TARGET)

