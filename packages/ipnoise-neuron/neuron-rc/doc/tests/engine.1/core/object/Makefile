DIRS = 	object			\
		dumper			\
		file			\
		map				\
		parser			\
		splitter

BUILDDIRS	= $(DIRS:%=build-%)
INSTALLDIRS	= $(DIRS:%=install-%)
CLEANDIRS	= $(DIRS:%=clean-%)
TESTDIRS	= $(DIRS:%=test-%)

.PHONY: subdirs $(DIRS)
.PHONY: subdirs $(BUILDDIRS)
.PHONY: subdirs $(INSTALLDIRS)
.PHONY: subdirs $(TESTDIRS)
.PHONY: subdirs $(CLEANDIRS)
.PHONY: default all install clean

default: all

## build
all: $(BUILDDIRS)
$(DIRS): $(BUILDDIRS)
$(BUILDDIRS):
	@$(MAKE) -C $(@:build-%=%)

## install
install: $(INSTALLDIRS) all
$(INSTALLDIRS):
	@$(MAKE) -C $(@:install-%=%) install

## clean
clean: $(CLEANDIRS)
$(CLEANDIRS):
	@$(MAKE) -C $(@:clean-%=%) clean

