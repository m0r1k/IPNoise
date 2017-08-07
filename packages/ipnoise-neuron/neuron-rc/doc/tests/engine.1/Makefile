TARGET		= engine
CROSS		=
LD			= gcc
CC			= gcc

OPT			= -g3 -O0
#OPT			= -g3 -O3
CFLAGS		= $(OPT) -fPIC -Wall -Werror
LDFLAGS 	=
INCLUDE		= -I./include/						\
	-I/mnt/multimedia/git/IPNoise/packages/		\
	-I/usr/include/glib-2.0/					\
	-I/usr/lib64/glib-2.0/include/

LIBS		= -ldl -lglib-2.0

DIRS		= .
CFILES  	=  $(foreach dir,$(DIRS),$(wildcard $(dir)/*.c))
OFILES		+= $(CFILES:.c=.o)
INCLUDE		+= $(foreach dir,$(DIRS),-I$(CURDIR)/$(dir))

.PHONY: default clean modules ctags

default: ctags $(TARGET) modules

$(TARGET): $(OFILES)
	@echo built ... $@
	$(CROSS)$(LD) $(LDFLAGS) $(OFILES) -o $@ $(LIBS)

%.o: %.c
	@echo $<
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

modules:
	@mkdir -p modules
	@make  -C core/object/ install DEST=../../../modules

ctags:
	@rm -rf tags
	ctags -R .

clean:
	rm -f *.o
	rm -f $(TARGET)
	make -C core/object/ clean

