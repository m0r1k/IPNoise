TARGET		= engine
CROSS		=
LD			= gcc
CC			= gcc

OPT			= -g3 -O0
#OPT			= -g3 -O3
CFLAGS		= $(OPT) -fPIC -Wall -Werror -D_GNU_SOURCE -D_EVENT_DISABLE_THREAD_SUPPORT
LDFLAGS 	= -L/mnt/multimedia/git/IPNoise/packages/ipnoise-libevent/.files/usr/lib/
INCLUDE		= -I./include/						\
	-I/mnt/multimedia/git/IPNoise/packages/		\
	-I/mnt/multimedia/git/IPNoise/packages/ipnoise-libevent/libevent/include/	\
	-I/usr/include/glib-2.0/					\
	-I/usr/lib/glib-2.0/include/

LIBS		= -ldl -lglib-2.0 -levent -lpthread

DIRS		= .
CFILES  	=  $(foreach dir,$(DIRS),$(wildcard $(dir)/*.c))
OFILES		+= $(CFILES:.c=.o)
INCLUDE		+= $(foreach dir,$(DIRS),-I$(CURDIR)/$(dir))

.PHONY: default clean ctags

default: ctags $(TARGET)

$(TARGET): $(OFILES)
	@echo built ... $@
	$(CROSS)$(LD) $(LDFLAGS) $(OFILES) -o $@ $(LIBS)

%.o: %.c
	@echo $<
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

ctags:
	@rm -rf tags
	ctags -R .

clean:
	rm -f *.o
	rm -f $(TARGET)

