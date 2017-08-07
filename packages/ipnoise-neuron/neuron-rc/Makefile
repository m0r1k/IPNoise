DEFINES +=
OFLAGS  ?= -g3 -O0
#OFLAGS  ?= -g0 -O3
INCLUDE += -I.										\
	-I../../ 										\
	-Icore/object/object/src/						\
	-I/mnt/multimedia/git/IPNoise/packages/

CFLAGS  = -std=gnu++0x								\
	-fno-exceptions									\
	$(INCLUDES)										\
	$(DEFINES)										\
	$(OFLAGS)										\
	-fPIC -Wall -Werror

LDFLAGS =
LIBS 	=

## linux
linux: CROSS     =
linux: CC        = gcc
linux: CXX       = g++
linux: CFLAGS   += -DLINUX -Dusemultiplicity -pthread
linux: CXXFLAGS += $(CFLAGS)
linux: CCFLAGS  += $(CFLAGS)
linux: LDFLAGS  += -L.libs/
linux: LIBS 	+= -ldl -lpthread

# Source folders and executable name
DIRS   = src
TARGET = ./$(shell basename $(CURDIR))

SOURCES  += $(DIRS)

# Source files
CPPFILES :=  $(foreach dir,$(SOURCES),$(wildcard $(dir)/*.cpp))
CFILES   :=  $(foreach dir,$(SOURCES),$(wildcard $(dir)/*.c))

# Use CXX for linking C++ projects, CC for standard C
ifeq ($(strip $(CPPFILES)),)
    export LD   :=  $(CC)
else
    export LD   :=  $(CXX)
endif

OFILES 		+=  $(CPPFILES:.cpp=.o) $(CFILES:.c=.o)
INCLUDE 	+=  $(foreach dir,$(DIRS),-I$(CURDIR)/$(dir))

.PHONY: default linux clean modules test1

default: linux

linux: $(TARGET) modules

$(TARGET): $(OFILES)
	@echo built ... $@
	$(CROSS)$(LD) $(LDFLAGS) $(OFILES) -o $@ $(LIBS)

%.o: %.cpp
	@echo $<
	$(CROSS)$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

%.o: %.c
	@echo $<
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

modules:
	mkdir -p modules/
	make  -C core/object/

test1:
	## start thread
	$(TARGET) -c core.object.thread -p /db/objects/static.thread.morik -s -t /db/threads/
	## create string 1
	$(TARGET) -c core.object.string -p /db/objects/static.string1.morik -i "it's string 1" -s
	## create string 2, add to actions of string 1, add link /db/objects/static.string1.morik
	$(TARGET) -c core.object.string -p /db/objects/static.string2.morik -i "it's string 2" -a /db/objects/static.string1.morik -s -t /db/objects/static.string1.morik/actions
	## add string 1 to actions of thread
	$(TARGET) -c core.object.string -p /db/objects/static.string1.morik -t /db/actions

test2:
	## create string 1
	$(TARGET) -c core.object.string -p /db/objects/static.string1.morik -i "it's string 1" -s
	## create thread, add link to string 1
	$(TARGET) -c core.object.thread -p /db/objects/static.thread.morik -a /db/objects/static.string1.morik -s
	## start thread
	$(TARGET) -l/db/objects/static.thread.morik -t /db/threads/

test3:
	## create http server
	$(TARGET) -c core.object.http.server -p /db/objects/static.http.server -i "{ port: 80 }" -a /db/objects/static.thread.1 -s
	## create thread, add link to http server
	$(TARGET) -c core.object.thread -p /db/objects/static.thread.1 -a /db/objects/static.http.server -s
	## start thread
	$(TARGET) -l/db/objects/static.thread.1 -t /db/threads/
	## request content
	##GET http://192.168.223.128:8080/

clean:
	@find $(DIRS) -name "*.o" | xargs -i rm -f {}
	@rm -fr $(TARGET)
	@make -C core/object/ clean
	@rm -rf modules

