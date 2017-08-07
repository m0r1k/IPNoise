DEST    = ../../modules/
TARGETS =	$(DEST)core.object.object.so				\
			$(DEST)core.object.dir.so					\
			$(DEST)core.object.double.so				\
			$(DEST)core.object.file.so					\
			$(DEST)core.object.inotify.so				\
			$(DEST)core.object.int32.so					\
			$(DEST)core.object.links.so					\
			$(DEST)core.object.map.so					\
			$(DEST)core.object.microcode.so				\
			$(DEST)core.object.microcode.location.so	\
			$(DEST)core.object.http.request.so			\
			$(DEST)core.object.http.server.so			\
			$(DEST)core.object.string.so				\
			$(DEST)core.object.dummy.so					\
			$(DEST)core.object.thread.so				\
			$(DEST)core.object.uint32.so				\
			$(DEST)core.object.unique.so				\
			$(DEST)core.object.user.so					\
			$(DEST)core.object.vector.so

CPPFILES =  object/main.cpp								\
			dir/main.cpp								\
			double/main.cpp								\
			file/main.cpp								\
			inotify/main.cpp							\
			int32/main.cpp								\
			links/main.cpp								\
			microcode/main.cpp							\
			microcodeLocation/main.cpp					\
			map/main.cpp								\
			http.request/main.cpp						\
			http.server/main.cpp						\
			string/main.cpp								\
			dummy/main.cpp								\
			thread/main.cpp								\
			uint32/main.cpp								\
			unique/main.cpp								\
			user/main.cpp								\
			vector/main.cpp

CFILES =

DEFINES 	+=
DEST		?= ../../modules/
OFLAGS  	?= -g3 -O0
#OFLAGS  	?= -g0 -O3
INCLUDE 	+= -I.										\
	-I../ 												\
	-I../../											\
	-I../../src/										\
	-I../../../../										\
	-I../../../../../									\
	-I../../../../ipnoise-libevent/libevent/include/	\
	-I/mnt/multimedia/git/IPNoise/packages/				\
	-I/mnt/multimedia/git/IPNoise/packages/ipnoise-libevent/libevent/include/

CFLAGS  	+= -std=gnu++0x								\
	-fno-exceptions										\
	$(INCLUDE)											\
	$(DEFINES)											\
	$(OFLAGS)											\
	-fPIC -Wall -Werror

CROSS    ?=
CC       ?= gcc
CXX      ?= g++
CXXFLAGS += $(CFLAGS)
CCFLAGS  += $(CFLAGS)
LDFLAGS  += -shared $(OFLAGS)
LIBS 	 += -ldl -levent -L/mnt/multimedia/git/IPNoise/packages/ipnoise-libevent/.files/usr/lib/

# Use CXX for linking C++ projects, CC for standard C
ifeq ($(strip $(CPPFILES)),)
    export LD   :=  $(CC)
else
    export LD   :=  $(CXX)
endif

OFILES += $(CPPFILES:.cpp=.o) $(CFILES:.c=.o)

.PHONY: default clean

default: $(TARGETS)

%.o: %.cpp
	@echo $<
	$(CROSS)$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	@echo $<
	$(CROSS)$(CC) $(CFLAGS) -c $< -o $@

%.so: $(OFILES)
	@echo "OFILES: '$(OFILES)'"
	@echo "build: '$@'"
	$(CROSS)$(LD) $(LDFLAGS) $(OFILES) $(LIBS) -o $@

clean:
	@rm -f $(OFILES)
	@rm -f $(TARGETS)

