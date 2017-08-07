DEFINES = -DQT_NO_DEBUG -DQT_GUI_LIB -DQT_CORE_LIB -DQT_SHARED
CFLAGS  = -g3 -O0 -Wall $(DEFINES) 			\
	-I../../../ 							\
	-I/usr/lib/qt4/include/ 				\
	-I/usr/lib/qt4/include/QtCore/ 			\
	-I/usr/lib/qt4/include/QtGui/			\
	-I/usr/lib/qt4/include/QtNetwork/

#	-finput-charset=utf8

LDFLAGS = -g3 -finput-charset=utf8
LIBS 	= ../../.files/packer.a

ARCHES	= lin32 win32 arm32

## linux 32
lin32: CROSS     =
lin32: CC        = gcc
lin32: CXX       = g++
## mandriva ln -s /usr/lib/qt4/bin/moc /usr/bin/moc-qt4
lin32: MOC       = moc-qt4
lin32: CFLAGS   += -DLIN32
lin32: CXXFLAGS += $(CFLAGS)
lin32: CCFLAGS  += $(CFLAGS)
lin32: LDFLAGS  +=
lin32: LIBS 	+= -lQtGui -lQtCore -lQtNetwork ../../.files/liblzma.a

## win 32
win32: CROSS    = i686-w64-mingw32-
win32: CC       = gcc
win32: CXX      = g++
win32: MOC      = moc
win32: CFLAGS   += -DWIN32 		\
		-DUNICODE -D_UNICODE	\
		`$(CROSS)pkg-config --cflags gthread-2.0`
win32: CXXFLAGS += $(CFLAGS)
win32: CCFLAGS  += $(CFLAGS)
win32: LDFLAGS  +=
win32: LIBS 	+= -lQtGui4 -lQtCore4 -lQtNetwork4 -llzma -liconv

## arm 32
arm32: CROSS     =
arm32: CC        = gcc
arm32: CXX       = g++
arm32: MOC       = moc-qt4
arm32: CFLAGS   += -DARM32	\
	-mcpu=cortex-a8 -mtune=cortex-a8 		\
	-fomit-frame-pointer -fsigned-char 		\
	-mfpu=neon -mfloat-abi=softfp
arm32: CXXFLAGS += $(CFLAGS)
arm32: CCFLAGS  += $(CFLAGS)
arm32: LDFLAGS  +=
arm32: LIBS 	+= -lQtGui -lQtCore -lQtNetwork

TARGET = $(shell basename $(CURDIR))

# Some more include paths
INCPATHS:=  -I/usr/share/qt4/mkspecs/default 	\
	-I/usr/include/QtGui						\
	-I/usr/include/QtXml						\
	-I/usr/include/QtCore						\
	-I/usr/include/QtNetwork					\
	-I../../

# Source folders and executable name
DIRS = src					\
	src/wizards				\
	src/wizards/install

SOURCES := $(DIRS)
INCLUDES:= $(DIRS)

# Source files
export VPATH    :=  $(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
                    $(foreach dir,$(INCLUDES),$(CURDIR)/$(dir))

CPPFILES    :=  $(foreach dir,$(SOURCES),$(wildcard $(dir)/*.cpp))
HPPFILES    :=  $(foreach dir,$(INCLUDES),$(wildcard $(dir)/*.hpp))

# Use CXX for linking C++ projects, CC for standard C
ifeq ($(strip $(CPPFILES)),)
    export LD   :=  $(CC)
else
    export LD   :=  $(CXX)
endif

export OFILES   :=  $(HPPFILES:.hpp=.moc.o) $(CPPFILES:.cpp=.o)
export INCLUDE  :=  $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) $(INCPATHS)

.PHONY: usage lin32 win32 arm32 clean

usage:
	@echo "Compilation: make $(ARCHES)"
	@echo "Clean:       make clean"

lin32: $(TARGET)
win32: $(TARGET)
arm32: $(TARGET)

$(TARGET): $(OFILES) ../../.files/packer.a
	@echo built ... $@
	$(CROSS)$(LD) $(LDFLAGS) $(OFILES) -o $@ $(LIBS)

%.o: %.cpp
	@echo $<
	$(CROSS)$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

%.moc.cpp: %.hpp
	@echo $<
	$(CROSS)$(MOC) $< $(DEFINES) $(INCLUDE) -o $@

%.moc.o: %.moc.cpp
	@echo $<
	$(CROSS)$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

clean:
	@find . -name "*.o" 		| xargs -i rm -f {}
	@find . -name "*.moc.cpp" 	| xargs -i rm -f {}
	@rm -fr $(TARGET)

