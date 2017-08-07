TARGET	= ./server
#OFLAGS	= -O3 -g0
CCFLAGS	= $(OFLAGS)								\
	-fPIC -Wall -Werror							\
	-I../../									\
	-I/mnt/multimedia/git/IPNoise/packages/ipnoise-libevent/.files/usr/include/ \
	-I/mnt/multimedia/git/IPNoise/packages/		\
	-I/home/morik/CImg-1.5.8_rolling140127/
LDFLAGS	= -lssl	-lcrypto						\
 	-lX11 -lpng	-ljpeg							\
	-levent -L/mnt/multimedia/git/IPNoise/packages/ipnoise-libevent/.files/usr/lib/

DIRS    = ./
SOURCES	= $(DIRS)
CFILES 	= $(foreach dir,$(SOURCES),$(wildcard $(dir)/*.c))
HFILES 	= $(foreach dir,$(SOURCES),$(wildcard $(dir)/*.h))
OFILES 	= $(CFILES:.c=.o)
LD		= gcc
CC		= gcc

.PHONY: default clean

default: $(TARGET) tags

$(TARGET):	$(OFILES)
	@echo built ... $@
	$(CROSS)$(LD) $(LDFLAGS) $(OFILES) -o $@ $(LIBS)

%.c: $(HFILES)

%.o: %.c
	@echo $<
	$(CROSS)$(CC) $(CCFLAGS) $(INCLUDE) -c $< -o $@

tags: $(CFILES) $(HFILES)
	@echo recreating $@
	@rm -f $@
	@ctags -R .

clean:
	rm -f *.o
	rm -f $(TARGET)

