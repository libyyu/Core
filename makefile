CC = g++
CFLAGS = -std=c++0x -g -Wno-deprecated
INCLUDE = -I.
LIBRARY = #-lstdc++ -lpthread
BOOST_SDK = -I/Volumes/SHARED/WorkSpace/boost_1_67_0
BOOST_LIB = -L/Volumes/SHARED/WorkSpace/boost_1_67_0/macosx/static_lib
BOOST_CFLAGS = -lboost_regex
LUA_INCLUDE = -I/Volumes/SHARED/WorkSpace/AzureEngine/lua51/src
LUA_LIB = -L/Volumes/SHARED/WorkSpace/AzureEngine/Projects/macosx/x86_64
LUA_CFLAGS = -llua51

all : build testnet boosttest 3rdtest

build : test.o
	$(CC) $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIBRARY) -D_RZ_USE_MEMTRACK
test.o : rzlib/base/test.cpp
	$(CC) -c $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIBRARY) -D_RZ_USE_MEMTRACK
testnet : test2.o
	$(CC) $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIBRARY)
test2.o : rzlib/net/test.cpp
	$(CC) -c $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIBRARY)
boosttest : boosttest.o
	$(CC) $(CFLAGS) $(BOOST_CFLAGS) -o $@ $^ $(BOOST_SDK) $(BOOST_LIB)
boosttest.o : boosttest.cpp
	$(CC) -c $(CFLAGS) -o $@ $^ $(BOOST_SDK)
3rdtest : 3rdtest.o
	$(CC) $(CFLAGS) $(LUA_CFLAGS) -o $@ $^ $(LUA_INCLUDE) $(LUA_LIB)
3rdtest.o : rzlib/3rd/3rdtest.cpp
	$(CC) -c $(CFLAGS) -o $@ $^ $(LUA_INCLUDE)
clean :
	rm -rf *.o build testnet boosttest 3rdtest