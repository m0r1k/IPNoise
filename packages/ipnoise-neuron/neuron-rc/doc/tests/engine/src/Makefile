TARGET		= engine
CROSS		=
LD			= g++
CC			= g++

OPT			= -g3 -O0
#OPT		= -g3 -O3
CFLAGS		= $(OPT) -fPIC -Wall -Werror -std=c++0x
#-D_EVENT_DISABLE_THREAD_SUPPORT
LDFLAGS 	= -L/mnt/multimedia/git/IPNoise/packages/ipnoise-libevent/.files/usr/lib/
INCLUDE		= -I./include/						\
	-I/mnt/multimedia/git/IPNoise/packages/		\
	-I/mnt/multimedia/git/IPNoise/packages/ipnoise-libevent/libevent/include/

LIBS		= -levent -lpthread	\
	-lv8
# -lv8_base -lv8_libplatform

#g++ -I../ shell.cc -o shell -lv8 -pthread -std=c++0x -L../out/x64.release/obj.target/tools/gyp/ -lv8_base -l v8_libplatform

DIRS		= .
CPPFILES  	=  $(foreach dir,$(DIRS),$(wildcard $(dir)/*.cpp))
OFILES		+= $(CPPFILES:.c=.o)
INCLUDE		+= $(foreach dir,$(DIRS),-I$(CURDIR)/$(dir))

.PHONY: default clean ctags

default: ctags $(TARGET)

$(TARGET): $(OFILES)
	@echo built ... $@
	$(CROSS)$(LD) $(LDFLAGS) $(CFLAGS) $(OFILES) -o $@ $(LIBS)

%.o: %.cpp
	@echo $<
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

ctags:
	@rm -rf tags
	ctags -R .

clean:
	rm -f *.o
	rm -f $(TARGET)
	rm -f tags


