DESTDIR=.libs

.PHONY: default clean fftreal install

default: $(DESTDIR)/libfftreal.so

fftreal/Makefile:
	cd fftreal && qmake-qt5

$(DESTDIR)/libfftreal.so: fftreal/Makefile
	make -C fftreal

install:
	mkdir -p $(DESTDIR)
	cp -f fftreal/*.so* $(DESTDIR)/

clean: fftreal/Makefile
	make -C fftreal clean
	rm -rf fftreal/*.so*

