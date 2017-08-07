ARCHES=lin32 win32 arm32

lin32: cross_prefix	=
lin32: DEFINES		= -DLIN32
lin32: CC			= g++
lin32: CCFLAGS 		= -I ../../ $(DEFINES)
lin32: LDFLAGS 		=
lin32: TARGET 		= ipnoise

win32: cross_prefix	= i686-w64-mingw32-
win32: DEFINES		= -DWIN32
win32: CC 			= g++
win32: CCFLAGS		= -I ../../ `pkg-config --cflags gthread-2.0` $(DEFINES)
win32: LDFLAGS		= `pkg-config --libs glib-2.0`
win32: TARGET		= ipnoise.exe

arm32: cross_prefix	=
arm32: DEFINES		= -DARM32
arm32: CC			= g++
arm32: CCFLAGS 		= -I ../../ $(DEFINES)
arm32: LDFLAGS 		=
arm32: TARGET 		= ipnoise

.PHONY = usage lin32 win32 arm32 clean

usage:
	@echo "Compilation: make $(ARCHES)"
	@echo "Clean:       make clean"

lin32: $(TARGET)
	${cross_prefix}$(CC) $(CCFLAGS) $(LDFLAGS) -o $(TARGET) src/main.cpp;

win32: $(TARGET)
	PATH=/usr/i686-w64-mingw32/sys-root/mingw/bin/:$$PATH; 					\
	PKG_CONFIG_PATH=/usr/i686-w64-mingw32/sys-root/mingw/lib/pkgconfig/;	\
	export PATH;															\
	export PKG_CONFIG_PATH;													\
	${cross_prefix}$(CC) $(CCFLAGS) $(LDFLAGS) -o $(TARGET) src/main.cpp;

arm32: $(TARGET)
	${cross_prefix}$(CC) $(CCFLAGS) $(LDFLAGS) -o $(TARGET) src/main.cpp;

clean:
	@rm -f src/*.o
	@rm -f ipnoise.exe
	@rm -f ipnoise

