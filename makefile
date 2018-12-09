CC = g++
CFLAGS = -std=c++0x -g -Wno-deprecated -fpermissive -Wconversion-null
INCLUDE = -Iflib -Iflib/base 
LIBRARY = #-lstdc++ -lpthread
BOOST_SDK = -I/Volumes/SHARED/WorkSpace/boost_1_67_0
BOOST_LIB = -L/Volumes/SHARED/WorkSpace/boost_1_67_0/macosx/static_lib
BOOST_CFLAGS = -lboost_regex
LUA_INCLUDE = -I/Volumes/SHARED/WorkSpace/AzureEngine/lua51/src
LUA_LIB = -L/Volumes/SHARED/WorkSpace/AzureEngine/Projects/macosx/x86_64
LUA_CFLAGS = -llua51

all : test_base test_process test_net test_boost test_3rd

test_base : test_base.o
	$(CC) $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIBRARY) -D_F_USE_MEMTRACK
test_base.o : flib/example/test_base.cpp
	$(CC) -c $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIBRARY) -D_F_USE_MEMTRACK

test_process : test_process.o
	$(CC) $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIBRARY)
test_process.o : flib/example/test_process.cpp
	$(CC) -c $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIBRARY)

test_net : test_net.o
	$(CC) $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIBRARY)
test_net.o : flib/example/test_net.cpp
	$(CC) -c $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIBRARY)

test_boost : test_boost.o
	$(CC) $(CFLAGS) $(BOOST_CFLAGS) -o $@ $^ $(BOOST_SDK) $(BOOST_LIB)
test_boost.o : flib/example/test_boost.cpp
	$(CC) -c $(CFLAGS) -o $@ $^ $(BOOST_SDK)

test_3rd : test_3rd.o
	$(CC) $(CFLAGS) $(LUA_CFLAGS) -o $@ $^ $(LUA_INCLUDE) $(LUA_LIB)
test_3rd.o : flib/example/test_3rd.cpp
	$(CC) -c $(CFLAGS) -o $@ $^ $(INCLUDE) $(LUA_INCLUDE)
clean :
	rm -rf *.o test_base test_process test_net test_boost test_3rd