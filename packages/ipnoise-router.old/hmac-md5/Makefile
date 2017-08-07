##
## Roman E. Chechnev (c) 2009 <ipnoise@chechnev.ru>
## IPNoise HUID generator
##

default: ctags ipnoise-huid-gen cptHMAC.o cptMD5.o cptHash.o cptBase64.o cptBase16.o gen-huid.o

ctags:
	rm -f tags
	ctags -R .

cptHMAC.o: cptHMAC.cpp cptHMAC.h
	c++ -c -o $@ $(@:.o=.cpp)

cptMD5.o: cptMD5.cpp cptMD5.h
	c++ -c -o $@ $(@:.o=.cpp)

cptHash.o: cptHash.cpp cptHash.h
	c++ -c -o $@ $(@:.o=.cpp)

cptBase64.o: cptBase64.cpp cptBase64.h
	c++ -c -o $@ $(@:.o=.cpp)

cptBase16.o: cptBase16.cpp cptBase16.h
	c++ -c -o $@ $(@:.o=.cpp)

gen-huid.o: gen-huid.cpp
	c++ -c -o $@ $(@:.o=.cpp)

ipnoise-huid-gen: ipnoise-huid-gen.o cptHMAC.o cptMD5.o cptHash.o cptBase64.o cptBase16.o gen-huid.o
	c++ -o $@ *.o
	rm ipnoise-huid-gen.o

ipnoise-huid-gen.o: ipnoise-huid-gen.cpp
	c++ -c -o $@ $(@:.o=.cpp)

clean:
	rm -f *.o tags ipnoise-huid-gen

