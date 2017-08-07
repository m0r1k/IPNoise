DEFINES =
CFLAGS  = -g3 -O0 -Wall $(DEFINES) -I ../../
LDFLAGS = -g3
LIBS 	= -L../.files/usr/lib/ -levent

CROSS     =
CC        = gcc
CXX       = g++
CFLAGS    = -DPLATFORM=$(PLATFORM) -D_LIN32
CXXFLAGS += $(CFLAGS)
CCFLAGS  += $(CFLAGS)
LDFLAGS  +=
LIBS     +=

TARGET = $(shell basename $(CURDIR))

# Some more include paths
INCPATHS:= -I../../	\
	-I../../ipnoise-libevent/libevent/include/

# Source folders and executable name
DIRS =  src					\
	src/common 				\
	src/tcp_v4/				\
	src/udp_v4/

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

.PHONY: all prepare install clean

all: prepare install

prepare:

install: $(TARGET)

$(TARGET): $(OFILES)
	@echo built ... $@
	$(CROSS)$(LD) $(LDFLAGS) $(OFILES) -o $@ $(LIBS)

%.o: %.cpp
	@echo $<
	$(CROSS)$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

clean:
	@find . -name "*.o" | xargs -i rm -f {}
	@rm -fr $(TARGET)

