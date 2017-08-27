## for debug rpms, don't forget install: 'redhat-rpm-config'

.PHONY += usage prepare lin32 lin64 win32 arm32 make install rpms clean

ARCHES ?= lin32 lin64 win32 arm32
HASH_PATHS += .arch

usage:
	@echo "Compilation: make $(ARCHES)"
	@echo "Clean:       make clean"

prepare:
	@## app name
	@if [ ! -n "$(APPNAME)" ];then											\
		echo "Unknown APPNAME";												\
		exit 1;																\
	fi
	@## app dir
	@if [ -n "$(APPDIR)" ];then												\
		rm -f $(APPNAME);													\
		ln -sf $(APPDIR) $(APPNAME);										\
	fi
	@## rpms dir
	@if [ -n "$(STORE_RPMS_TO)" ];then										\
		rm -rf $(STORE_RPMS_TO);											\
		install -m 0755 -d $(STORE_RPMS_TO);								\
	fi
	@## output files dir
	@#@if [ -n "$(STORE_FILES_TO)" ];then
	@#	rm -rf $(STORE_FILES_TO);
	@#	install -m 0755 -d $(STORE_FILES_TO);
	@#fi

$(ARCHES): prepare
	@touch .arch;													\
		ARCH="`cat .arch`";											\
		if [ "$$ARCH" != "$@" ];then								\
			$(MAKE) $(APPNAME)-clean-$@	|| exit 1;					\
			$(MAKE) $(APPNAME)-platform-changed	|| exit 1;			\
			echo $@ > .arch;										\
		fi
	@if [ -n "$(HASH_PATHS)" ];then									\
		find $(HASH_PATHS)											\
			| sort -u												\
			| xargs -e ls -l 										\
			| openssl dgst -hex -md5 								\
			| awk '{print $$2}' > target.hash;						\
	else															\
		rm -f target.hash last_build.hash;							\
	fi;																\
	TARGET_HASH="";													\
	if [ -r "target.hash" ];then									\
		TARGET_HASH=`cat target.hash`;								\
	fi; 															\
	LAST_BUILD_HASH="";												\
	if [ -r "last_build.hash" ];then								\
		LAST_BUILD_HASH=`cat last_build.hash`;						\
	fi;																\
	if 		[ ! -n "$$TARGET_HASH" 		]							\
		|| 	[ ! -n "$$LAST_BUILD_HASH" 	]							\
		|| 	[ "$$TARGET_HASH" != "$$LAST_BUILD_HASH" ];				\
	then															\
		$(MAKE) $(APPNAME)-prepare-$@ 	|| exit 1;					\
		$(MAKE) $(APPNAME)-make-$@ 		|| exit 1;					\
		if [ -n "$(HASH_PATHS)" ];then								\
			find $(HASH_PATHS)										\
				| sort -u											\
				| xargs -e ls -l 									\
				| openssl dgst -hex -md5 							\
				| awk '{print $$2}' > last_build.hash;				\
		fi;															\
	fi;

install:
	@ARCH=`if [ -r ".arch" ]; then cat .arch; fi`;					\
		if [ -n "$$ARCH" ];then										\
			$(MAKE) $(APPNAME)-install-$$ARCH;						\
		else														\
			echo "nothing to install, run make before";				\
			exit 1;													\
		fi

rpms: prepare
	@if [ ! -n "$(RPMS_DIR)" ];then			\
		echo "RPMS_DIR not defined";		\
		exit 1;								\
	fi
	@if [ ! -n "$(SRPMS_DIR)" ];then		\
		echo "SRPMS_DIR not defined";		\
		exit 1;								\
	fi
	@if [ ! -n "$(STORE_RPMS_TO)" ];then	\
		echo "STORE_RPMS_TO not defined";	\
		exit 1;								\
	fi
	@$(MAKE) $(APPNAME)-rpms
	rm -f $(RPMS_DIR)/ipnoise-$(APPNAME)*
	rm -f $(SRPMS_DIR)/ipnoise-$(APPNAME)*
	rpmbuild -bb --target $(IPNOISE_PACKAGES_ARCH) ipnoise-$(APPNAME).spec	\
		--define "ipnoise_packages_path $(IPNOISE_PACKAGES_PATH)"			\
		--define "ipnoise_packages_arch $(IPNOISE_PACKAGES_ARCH)"			\
		--define "ipnoise_packages_target $(IPNOISE_PACKAGES_TARGET)"
	rm -rf .rpms
	install -m 0755 -d						$(STORE_RPMS_TO)
	cp $(RPMS_DIR)/ipnoise-$(APPNAME)* 		$(STORE_RPMS_TO)

clean:
	@rm -f target.hash last_build.hash
	@ARCH=`if [ -r ".arch" ]; then cat .arch; fi`;	\
		if [ -n "$$ARCH" ];then						\
			$(MAKE) $(APPNAME)-clean-$$ARCH;		\
		fi
	@rm -f .arch

