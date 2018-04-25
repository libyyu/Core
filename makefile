CC = g++
CFLAGS = -std=c++0x -g -Wno-deprecated
INCLUDE = -I.
LIBRARY = #-lstdc++ -lpthread
BOOST_SDK = -I/Volumes/SHARED/WorkSpace/boost_1_67_0
BOOST_LIB = -L/Volumes/SHARED/WorkSpace/boost_1_67_0/macosx/static_lib
BOOST_CFLAGS = -lboost_regex

all : build testnet boosttest

build : test.o
	$(CC) $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIBRARY)
test.o : rzlib/base/test.cpp
	$(CC) -c $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIBRARY)
testnet : test2.o
	$(CC) $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIBRARY)
test2.o : rzlib/net/test.cpp
	$(CC) -c $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIBRARY)
boosttest : boosttest.o
	$(CC) $(CFLAGS) $(BOOST_CFLAGS) -o $@ $^ $(BOOST_SDK) $(BOOST_LIB)
boosttest.o : boosttest.cpp
	$(CC) -c $(CFLAGS) -o $@ $^ $(BOOST_SDK)
clean :
	rm -rf *.o build testnet boosttest