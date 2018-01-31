DEFINES =
CFLAGS  += -g3 -O0 -Wall $(DEFINES) 		\
	-I../../

LDFLAGS = -static -g3
LIBS 	= -L../.files/ -llzma -lpthread

ARCHES	= lin32 lin64 win32 arm32

## linux 32
lin32: CROSS     =
lin32: CC        = gcc
lin32: CXX       = g++
lin32: CFLAGS   += -DLIN32
lin32: CXXFLAGS += $(CFLAGS)
lin32: CCFLAGS  += $(CFLAGS)
lin32: LDFLAGS  +=
lin32: LIBS 	+=

## linux 64
lin64: CROSS     =
lin64: CC        = gcc
lin64: CXX       = g++
lin64: CFLAGS   += -DLIN64
lin64: CXXFLAGS += $(CFLAGS)
lin64: CCFLAGS  += $(CFLAGS)
lin64: LDFLAGS  +=
lin64: LIBS 	+=

## win 32
win32: CROSS    = i686-w64-mingw32-
win32: CC       = gcc
win32: CXX      = g++
win32: CFLAGS   += -DWIN32 		\
		`$(CROSS)pkg-config --cflags gthread-2.0`
win32: CXXFLAGS += $(CFLAGS)
win32: CCFLAGS  += $(CFLAGS)
win32: LDFLAGS  +=
win32: LIBS 	+=

## arm 32
arm32: CROSS     =
arm32: CC        = gcc
arm32: CXX       = g++
arm32: CFLAGS   += -DARM32	\
	-mcpu=cortex-a8 -mtune=cortex-a8 		\
	-fomit-frame-pointer -fsigned-char 		\
	-mfpu=neon -mfloat-abi=softfp
arm32: CXXFLAGS += $(CFLAGS)
arm32: CCFLAGS  += $(CFLAGS)
arm32: LDFLAGS  +=
arm32: LIBS 	+=

TARGET = $(shell basename $(CURDIR))

# Some more include paths
INCPATHS := -I../../

# Source folders and executable name
DIRS = src

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

export OFILES   :=  $(CPPFILES:.cpp=.o)
export INCLUDE  :=  $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) $(INCPATHS)

.PHONY: usage lin32 lin64 clean

usage:
	@echo "Compilation: make $(ARCHES)"
	@echo "Clean:       make clean"

lin32: $(TARGET)
lin64: $(TARGET)
win32: $(TARGET)
arm32: $(TARGET)

$(TARGET): $(OFILES)
	@echo built ... $@
	$(CROSS)$(LD) $(LDFLAGS) $(OFILES) -o $@ $(LIBS)

%.o: %.cpp
	@echo $<
	$(CROSS)$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

clean:
	@find . -name "*.o" 		| xargs -i rm -f {}
	@rm -fr $(TARGET)

