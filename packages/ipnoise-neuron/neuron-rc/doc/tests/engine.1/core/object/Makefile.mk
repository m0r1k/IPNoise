ifndef TARGET
$(error TARGET is not set)
endif

ifndef CFILES
$(error CFILES is not set)
endif

DEST		?= ../../../modules
DEFINES 	+=
OFLAGS  	?= -g3 -O0
#OFLAGS  	?= -g0 -O3
INCLUDE 	+= -I../../../include	\
	-I/mnt/multimedia/git/IPNoise/packages/

CFLAGS  	+= $(INCLUDE)								\
	$(DEFINES)											\
	$(OFLAGS)											\
	-fPIC -Wall -Werror

CROSS    =
CC       = gcc
LD		 = gcc
CCFLAGS  += $(CFLAGS)
LDFLAGS  += -shared $(OFLAGS)
LIBS 	 +=

OFILES += $(CFILES:.c=.o)

.PHONY: default install clean

default: $(TARGET)

%.o: %.c
	@echo $<
	$(CROSS)$(CC) $(CFLAGS) -c $< -o $@

%.so: $(OFILES)
	@echo "OFILES: '$(OFILES)'"
	@echo "build: '$@'"
	$(CROSS)$(LD) $(LDFLAGS) $(OFILES) $(LIBS) -o $@

install: $(TARGET)
	cp -f $(TARGET) $(DEST)/

clean:
	@rm -f *.o
	@rm -f *.so

