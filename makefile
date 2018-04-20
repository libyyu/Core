CC = g++
CFLAGS = -std=c++0x -g
INCLUDE = -I.
LIBRARY = -lstdc++ -lpthread

all : base

base : test.o
	$(CC) $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIBRARY)
test.o : rzlib/base/test.cpp
	$(CC) -c $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIBRARY)
clean :
	rm -rf base *.o