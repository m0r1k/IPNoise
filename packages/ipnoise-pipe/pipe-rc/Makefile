DEFINES = -DQT_NO_DEBUG -DQT_GUI_LIB -DQT_CORE_LIB -DQT_SHARED
CFLAGS  = -fPIC -g3 -O0 -Wall $(DEFINES) 		\
	-I../../ 							\
	-I/usr/include/qt5

LDFLAGS = -g3
LIBS 	=
ARCHES	= lin32 win32 arm32

## win 32
win32: CROSS    = i686-w64-mingw32-
win32: CC       = gcc
win32: CXX      = g++
win32: MOC      = moc
win32: CFLAGS   += -DWIN32 		\
		`$(CROSS)pkg-config --cflags gthread-2.0` 	\
		-DLOG_DISABLE_COLOR_SUPPORT
win32: CXXFLAGS += $(CFLAGS)
win32: CCFLAGS  += $(CFLAGS)
win32: LDFLAGS  += `$(CROSS)pkg-config --libs glib-2.0`
win32: LIBS 	+= -lQtGui4 -lQtCore4 -lQtXml4 -lQtNetwork4 -lwinmm

## linux 32
lin32: CROSS     =
lin32: CC        = gcc
lin32: CXX       = g++
## mandriva ln -s /usr/lib/qt5/bin/moc /usr/bin/moc-qt5
lin32: MOC       = moc-qt5
lin32: CFLAGS   += -DLIN32
lin32: CXXFLAGS += $(CFLAGS)
lin32: CCFLAGS  += $(CFLAGS)
lin32: LDFLAGS  +=
lin32: LIBS 	+= -lQt5Gui -lQt5Core -lQt5Widgets -lQt5Xml -lQt5Network

## arm 32
arm32: CROSS     =
arm32: CC        = gcc
arm32: CXX       = g++
arm32: MOC       = moc-qt5
arm32: CFLAGS   += -DARM32	\
	-mcpu=cortex-a8 -mtune=cortex-a8 		\
	-fomit-frame-pointer -fsigned-char 		\
	-mfpu=neon -mfloat-abi=softfp
arm32: CXXFLAGS += $(CFLAGS)
arm32: CCFLAGS  += $(CFLAGS)
arm32: LDFLAGS  +=
arm32: LIBS 	+= -lQtGui -lQtCore -lQtXml -lQtNetwork

#-mcpu=cortex-a8 -mtune=cortex-a8 -fomit-frame-pointer -fsigned-char -mfpu=neon -mfloat-abi=softfp -march=armv7-a -mlittle-endian -mfpu=vfpv3 -mfloat-abi=softfp
##-march=armv7-a
#-mcpu=cortex-m3 -mthumb
#-march=armv7-a -mtune=cortex-a8 -mfpu=neon -ftree-vectorize -ffast-math -mfloat-abi=softfp
#--mcpu=cortex-a8 -mtune=cortex-a8 -fomit-frame-pointer -fsigned-char -mfpu=neon -mfloat-abi=softfp
#-mcpu=cortex-a8 -mtune=cortex-a8 -fomit-frame-pointer -fsigned-char -mfpu=neon -mfloat-abi=softfp -march=armv7-a -mlittle-endian -mfpu=vfpv3 -mfloat-abi=softfp
#-march=armv7-a

TARGET = $(shell basename $(CURDIR))

# Some more include paths
INCPATHS:=  -I/usr/share/qt5/mkspecs/default 	\
	-I/usr/include/QtGui						\
	-I/usr/include/QtXml						\
	-I/usr/include/QtCore						\
	-I/usr/include/QtNetwork					\
	-I../../

# Source folders and executable name
DIRS =  src					\
	src/api 				\
	src/api/commands 		\
	src/api/events 			\
	src/contactList 		\
	src/wizards 			\
	src/wizards/add			\
	src/wizards/addContact	\
	src/wizards/addLink		\
	src/wizards/addGroup	\
	src/wizards/delGroup	\
	src/wizards/searchUser	\
	src/links

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

$(TARGET): $(OFILES)
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

