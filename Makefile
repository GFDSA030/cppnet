ifeq ($(OS),Windows_NT)
CC = g++
IFILE = -ID:\dev\lib\mingw\include -Idepcode
LFILE = -LD:\dev\lib\mingw\lib
LIBS = -L. -lz -lssl -lcrypto -lCrypt32 -pthread -lws2_32 -Wl,-s $(LFILE)
CFLAG = -I. -Iinclude -Wall -O2 -std=c++20 $(IFILE)
AR = ar
else
CC = g++
IFILE = -Idepcode
LFILE =
LIBS = -L. -lz -lssl -lcrypto -pthread -Wl,-s $(LFILE)
CFLAG = -I. -Iinclude -Wall -O2 -std=c++20 $(IFILE)
AR = ar
endif

TARGETS2 = $(patsubst src/%.cpp, build/%.o, $(wildcard src/*.cpp))
DEPCODE = $(patsubst depcode/%.cpp, build/%.o, $(wildcard depcode/*.cpp))

all: main.out test2.out

main.out: build/main.o libunet.a libfragdeps.a
test2.out: build/test2.o libunet.a libfragdeps.a
udp.out: build/udptest.o libunet.a libfragdeps.a

libunet.a:$(TARGETS2)
libfragdeps.a:$(DEPCODE)
build/main.o: main.cpp
	@mkdir -p build
	$(CC) $< -c -o $@ $(CFLAG)
build/test_main.o: test_main.cpp
	@mkdir -p build
	$(CC) $< -c -o $@ $(CFLAG)
build/test2.o: test.cpp
	@mkdir -p build
	$(CC) $< -c -o $@ $(CFLAG)
build/udptest.o: udptest.cpp
	@mkdir -p build
	$(CC) $< -c -o $@ $(CFLAG)

build/%.o: src/%.cpp
	@mkdir -p build
	$(CC) $< -c -o $@ $(CFLAG)
build/%.o: depcode/%.cpp
	@mkdir -p build
	$(CC) $< -c -o $@ $(CFLAG)
%.out:
	$(CC) $^ -o $@ $(LIBS)
%.a:
	$(AR) rc $@ $^
	@mkdir -p lib
	cp $@ lib/
strip:
	strip *.out
clean:
	rm *.a *.out -r build lib
run: main.out
	./main.out

.PHONY:clean strip run
