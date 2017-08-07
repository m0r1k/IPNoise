DEFINES = -D_FILE_OFFSET_BITS=64

ifneq ($(DISABLE_HOSTOS),1)
DEFINES += -DHOSTOS
endif

CFLAGS  = -g3 -O0 -Wall $(DEFINES) -I ../../ -std=c++0x
LDFLAGS = -g3
LIBS 	= -lxml2 \
	-L../.files/usr/lib/ -levent -levent_core \
	-L../.files/usr/lib/ -lfuse				  \
	-L../.files/usr/lib/ -lnet

CROSS     =
CC        = gcc
CXX       = g++
CFLAGS   += -DLIN32
CXXFLAGS += $(CFLAGS)
CCFLAGS  += $(CFLAGS)
LDFLAGS  +=
LIBS     +=

TARGET = $(shell basename $(CURDIR))

# Some more include paths
INCPATHS:=  -I/usr/share/qt4/mkspecs/default 	\
	-I/usr/include/QtGui						\
	-I/usr/include/QtXml						\
	-I/usr/include/QtCore						\
	-I/usr/include/QtNetwork					\
	-I../../									\
	-I../../ipnoise-libevent/libevent/include/	\
	-I../../ipnoise-libfuse/libfuse/include/	\
	-I../../ipnoise-libnet/libnet/include/		\
	-I/usr/include/libxml2/

# Source folders and executable name
DIRS = 	src											\
		src/libxml2									\
		src/objects									\
		src/objects/client							\
		src/objects/client/handlerUnknown			\
		src/objects/client/handlerRaw				\
		src/objects/client/handlerTelnet			\
		src/objects/client/handlerTelnet/command	\
		src/objects/client/handlerHttp				\
		src/objects/api								\
		src/objects/api/event						\
		src/objects/api/event/link					\
		src/objects/api/event/ipnoise				\
		src/objects/api/command						\
		src/objects/api/command/client				\
		src/objects/api/command/link				\
		src/objects/api/command/ipnoise				\
		src/net

SOURCES 	:= $(DIRS)
INCLUDES	:= $(DIRS) src/hmac-md5

# Source files
export VPATH    :=  $(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
                    $(foreach dir,$(INCLUDES),$(CURDIR)/$(dir))

CPPFILES    :=  $(foreach dir,$(SOURCES),$(wildcard $(dir)/*.cpp))
HPPFILES    :=  $(foreach dir,$(INCLUDES),$(wildcard $(dir)/*.hpp))
HFILES   	:=  $(foreach dir,$(INCLUDES),$(wildcard $(dir)/*.h))

CPPFILES += src/hmac-md5/cptBase16.cpp 	\
	src/hmac-md5/cptBase64.cpp 			\
	src/hmac-md5/cptHash.cpp 			\
	src/hmac-md5/cptHMAC.cpp 			\
	src/hmac-md5/cptMD5.cpp 			\
	src/hmac-md5/gen-huid.cpp

HFILES += src/hmac-md5/cptBase16.h 		\
	src/hmac-md5/cptBase64.h 			\
	src/hmac-md5/cptHash.h 				\
	src/hmac-md5/cptHMAC.h 				\
	src/hmac-md5/cptMD5.h 				\
	src/hmac-md5/gen-huid.h

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
	@find . -name "*.o" 		| xargs -i rm -f {}
	@rm -fr $(TARGET)

