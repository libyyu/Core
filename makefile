CC = g++
CFLAGS = -std=c++0x -g -Wno-deprecated
INCLUDE = -I.
LIBRARY = #-lstdc++ -lpthread

all : build testnet

build : test.o
	$(CC) $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIBRARY)
test.o : rzlib/base/test.cpp
	$(CC) -c $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIBRARY)
testnet : test2.o
	$(CC) $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIBRARY)
test2.o : rzlib/net/test.cpp
	$(CC) -c $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIBRARY)
clean :
	rm -rf *.o build testnet