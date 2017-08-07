##
## Roman E. Chechnev (c) 2009 <ipnoise@chechnev.ru>
## IPNoise HUID generator
##

default: ctags ipnoise-huid-gen cptHMAC.o cptMD5.o cptHash.o cptBase64.o cptBase16.o gen-huid.o

ctags:
	rm -f tags
	ctags -R .

cptHMAC.o: cptHMAC.cpp cptHMAC.h
	g++ -c -o $@ $(@:.o=.cpp)

cptMD5.o: cptMD5.cpp cptMD5.h
	g++ -c -o $@ $(@:.o=.cpp)

cptHash.o: cptHash.cpp cptHash.h
	g++ -c -o $@ $(@:.o=.cpp)

cptBase64.o: cptBase64.cpp cptBase64.h
	g++ -c -o $@ $(@:.o=.cpp)

cptBase16.o: cptBase16.cpp cptBase16.h
	g++ -c -o $@ $(@:.o=.cpp)

gen-huid.o: gen-huid.cpp
	g++ -c -o $@ $(@:.o=.cpp)

ipnoise-huid-gen: ipnoise-huid-gen.o cptHMAC.o cptMD5.o cptHash.o cptBase64.o cptBase16.o gen-huid.o
	g++ -o $@ *.o
	rm ipnoise-huid-gen.o

ipnoise-huid-gen.o: ipnoise-huid-gen.cpp
	g++ -c -o $@ $(@:.o=.cpp)

clean:
	rm -f *.o tags ipnoise-huid-gen

