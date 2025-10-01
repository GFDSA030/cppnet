ifeq ($(OS),Windows_NT)
CC = clang++
IFILE = -ID:\dev\lib\mingw\include
LFILE = -LD:\dev\lib\mingw\lib
LIBS = -L. -lssl -lcrypto -lCrypt32 -pthread -lws2_32 -Wl,-s -static -static-libgcc -static-libstdc++ -fuse-ld=lld $(LFILE)
MINLIB = -L. -lssl -lcrypto -lCrypt32 -pthread -lws2_32 -Wl,-s -fuse-ld=lld $(LFILE)
#add msys package
CFLAG = -I. -Iinclude -Wall -O2 -std=c++20 $(IFILE)
AR = ar
else
CC = clang++
IFILE =
LFILE =
LIBS = -L. -lssl -lcrypto -pthread -Wl,-s $(LFILE)
CFLAG = -I. -Iinclude -Wall -O2 -std=c++20 $(IFILE)
AR = ar
endif

TARGETS2 = $(patsubst src/%.cpp, build/%.o, $(wildcard src/*.cpp))

all: main.out test2.out

main.out: build/main.o libunet.a
test2.out: build/test2.o libunet.a
udp.out: build/udptest.o libunet.a

_min.out:build/main.o libunet.a
	$(CC) $^ -o $@ $(MINLIB)

libunet.a:$(TARGETS2)
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
