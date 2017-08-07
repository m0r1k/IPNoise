DEFINES = -DQT_NO_DEBUG -DQT_GUI_LIB -DQT_CORE_LIB -DQT_SHARED
CFLAGS  = -g3 -O0 -fPIC -Wall -Werror $(DEFINES)	\
	-I.												\
	-I../../ 										\
	-I../../ipnoise-libnet/.files/usr/include/		\
	-I/usr/include/      							\
	-I/usr/lib64/perl5/CORE/

LDFLAGS = -g3
LIBS 	= -lsqlite3

## linux
linux: CROSS     =
linux: CC        = gcc
linux: CXX       = g++
## mandriva ln -s /usr/lib/qt4/bin/moc /usr/bin/moc-qt4
linux: MOC       = moc-qt4
linux: CFLAGS   += -DLIN32 -std=c++0x -Dusemultiplicity
linux: CXXFLAGS += $(CFLAGS)
linux: CCFLAGS  += $(CFLAGS)
linux: LDFLAGS  += -L.libs/ -L../../ipnoise-libnet/.files/usr/lib/
linux: LIBS 	+= -lQtCore -lQtXml -lQtNetwork \
	-lmongoclient -lboost_thread -lboost_filesystem -lboost_program_options -lboost_system \
	-lperl `perl -MExtUtils::Embed -e ccopts -e ldopts` \
	-lnet

# Some more include paths
INCPATHS:=										\
	-I/usr/share/qt4/mkspecs/default 			\
	-I/usr/include/QtGui						\
	-I/usr/include/QtXml						\
	-I/usr/include/QtCore						\
	-I/usr/include/QtNetwork					\
	-I../../

# Source folders and executable name
DIRS = src						\
	src/thread  				\
	src/thread/worker  			\
	src/prop					\
	src/prop/neuron				\
	src/prop/neuron/link		\
	core/neuron/basket			\
	core/neuron/bookmarks		\
	core/neuron/console			\
	core/neuron/core			\
	core/neuron/dialog			\
	core/neuron/dialog/msg 		\
	core/neuron/dialogs			\
	core/neuron/dream			\
	core/neuron/dreamland		\
	core/neuron/dreams			\
	core/neuron/god				\
	core/neuron/ipnoise			\
	core/neuron/ipnoise/link	\
	core/neuron/ipnoise/links	\
	core/neuron/ipnoise/neigh	\
	core/neuron/ipnoise/neighs	\
	core/neuron/ipnoise/skb		\
	core/neuron/media			\
	core/neuron/neuron			\
	core/neuron/order			\
	core/neuron/orders			\
	core/neuron/param			\
	core/neuron/param/address	\
	core/neuron/param/group		\
	core/neuron/photo			\
	core/neuron/photos			\
	core/neuron/playlist		\
	core/neuron/product			\
	core/neuron/products		\
	core/neuron/session			\
	core/neuron/shop			\
	core/neuron/trash			\
	core/neuron/user			\
	core/neuron/users

ifdef HUMAN
TARGET  = human
DEFINES	+= 	-DUI -DHUMAN -DLOG_SPECTRUMANALYSER -DLOG_ENGINE -DSUPERIMPOSE_PROGRESS_ON_WAVEFORM -DSPECTRUM_ANALYSER_SEPARATE_THREAD -DQT_NO_DEBUG -DQT_MULTIMEDIA_LIB -DQT_GUI_LIB -DQT_CORE_LIB -DQT_SHARED

CFLAGS	+= 	-I/usr/include/QtOpenGL/						\
		 	-I/usr/include/GL/								\
			-I/usr/local/include/							\
			-I3rdparty/fftreal/
LIBS	+= 	-lQGLViewer -lQtOpenGL -lQtMultimedia -lQtGui -lQtCore 			\
			-lglut -lGLU -lGL -lm -lpthread	-lfftreal						\
 			-lopencv_core -lopencv_contrib -lopencv_video -lopencv_highgui
DIRS	+=	src/ui 							\
			src/human						\
			src/human/model					\
			src/human/nerves				\
			src/human/nerves/video			\
			src/human/nerves/audio
FFTREAL=.libs/libfftreal.so
else
TARGET = $(shell basename $(CURDIR))
endif

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

export OFILES   :=  $(HPPFILES:.hpp=.moc.o) $(CPPFILES:.cpp=.o)
export INCLUDE  :=  $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) $(INCPATHS)

.PHONY: linux clean ctags default

default: linux

linux: $(TARGET)

.libs/libfftreal.so:
	make -C 3rdparty install DESTDIR=../.libs/

ctags:
	ctags -R .

$(TARGET): $(OFILES) $(FFTREAL)
	@echo built ... $@
	$(CROSS)$(LD) $(LDFLAGS) $(OFILES) -o $@ $(LIBS)

%.o: %.cpp
	@echo $<
	$(CROSS)$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

%.moc.cpp: %.hpp
	@echo $<
	$(CROSS)$(MOC) $< $(DEFINES) $(INCLUDE) -o $@

%.moc.o: %.moc.cpp
	@echo $<
	$(CROSS)$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

clean:
	#@make -C 3rdparty clean
	@find . -name "*.o" 		| xargs -i rm -f {}
	@find . -name "*.moc.cpp" 	| xargs -i rm -f {}
	@rm -fr $(TARGET)

