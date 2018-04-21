CC = g++
CFLAGS = -std=c++0x -g
INCLUDE = -I.
LIBRARY = #-lstdc++ -lpthread

all : build

build : test.o
	$(CC) $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIBRARY)
	rm -rf *.o
test.o : rzlib/base/test.cpp
	$(CC) -c $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIBRARY)
clean :
	rm -rf build *.o