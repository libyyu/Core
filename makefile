# platform
PLAT 		:=$(if $(PLAT),$(PLAT),$(if ${shell uname | egrep -i linux},linux,))
PLAT 		:=$(if $(PLAT),$(PLAT),$(if ${shell uname | egrep -i darwin},macosx,))
PLAT 		:=$(if $(PLAT),$(PLAT),$(if ${shell uname | egrep -i cygwin},cygwin,))
PLAT 		:=$(if $(PLAT),$(PLAT),$(if ${shell uname | egrep -i mingw},mingw,))
PLAT 		:=$(if $(PLAT),$(PLAT),$(if ${shell uname | egrep -i windows},windows,))
PLAT 		:=$(if $(PLAT),$(PLAT),linux)

CC = g++
CFLAGS = -std=c++0x -g -Wno-deprecated -fpermissive -Wconversion-null
INCLUDE = -Iflib -Iflib/base 
LIBRARY = #-lstdc++ -lpthread
BOOST_SDK = -I/Volumes/SHARED/WorkSpace/boost_1_67_0
BOOST_LIB = -L/Volumes/SHARED/WorkSpace/boost_1_67_0/macosx/static_lib
BOOST_CFLAGS = -lboost_regex
LUA_INCLUDE = -I../lua-5.1.5/src
LUA_LIB = -L../libs/macosx/x86_64
LUA_CFLAGS = -llua51

ifeq ($(PLAT),mingw)
	CFLAGS += -D_WIN32_WINNT=0x0603
endif

all : test_base test_process test_net test_3rd test_sm test_sm

test_base : test_base.o
	@echo $(PLAT) link $@
	$(CC) $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIBRARY) -D_F_USE_MEMTRACK
test_base.o : flib/example/test_base.cpp
	@echo $(PLAT) compile $@
	$(CC) -c $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIBRARY) -D_F_USE_MEMTRACK

test_sm : test_sm.o
	@echo $(PLAT) link $@
	$(CC) $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIBRARY) -D_F_USE_MEMTRACK
test_sm.o : flib/example/test_sm.cpp
	@echo $(PLAT) compile $@
	$(CC) -c $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIBRARY) -D_F_USE_MEMTRACK

test_process : test_process.o
	@echo $(PLAT) link $@
	$(CC) $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIBRARY)
test_process.o : flib/example/test_process.cpp
	@echo $(PLAT) compile $@
	$(CC) -c $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIBRARY)

test_net : test_net.o
	@echo $(PLAT) link $@
	$(CC) $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIBRARY)
test_net.o : flib/example/test_net.cpp
	@echo $(PLAT) compile $@
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
	rm -rf *.o test_base test_process test_net test_boost test_3rd test_sm
	