//
//  main.cpp
//  ddddddd
//
//  Created by Dengfeng Li on 2017/11/26.
//  Copyright © 2017年 Dengfeng Li. All rights reserved.
//
#include "FBuffer.hpp"
#include "FString.hpp"
#include "FObserver.hpp"
#include "FConvert.hpp"
#include "FConsole.hpp"
#include "FFunc.hpp"
#include "FFile.hpp"
#include "FCounter.hpp"
#include "FThread.hpp"
#include "FMD5.hpp"
#include "FBase64.hpp"
#include "FLogFile.hpp"
#include "FPlugin.hpp"
#include "FMemTrack.hpp"
_FUsing(std)
_FUsing(FStd)
static FAutoFile fGlobalLog("log.txt");
static FMemWtacher __mem_watcher;
int CalcDistance(int x, int z, int x2, int z2)
{
    int d = x - x2;
    int d2 = z - z2;
    return d*d + d2*d2;
}

void foo()
{
    int i=0;
    while(++i < 10)
	    printf("this is a thread function output %d\r\n", FGetCurrentThreadId());
}

void test()
{
    F_CONSOLE_TRACE
}

void testbase64()
{
    const char* str = "helloworld";
    std::string code = F_BASE64_ENCODE(str);
    F_CONSOLE(DEBUG) << "base64:" << F_BASE64_ENCODE(str) << "," << F_BASE64_DECODE(code.c_str()) << endl;
}
void testmd5()
{
    F_LOGFILE_TRACE(fGlobalLog)
    char buff[200];
    FMD5String("hello world", buff);
    F_CONSOLE(DEBUG) << buff << endl;
    F_LOGFILE(DEBUG, fGlobalLog) << "test md5" << endl;
}

int main(int argc, const char * argv[]) {
    // insert code here...
    F_CONSOLE_TRACE
    F_LOGFILE_TRACE(fGlobalLog)
    F_LOGFILE(DEBUG, fGlobalLog) << "LOGFILE" << endl;

    int * pT = new int(8);
    std::cout << "Hello, World!\n";
#if PLATFORM_TARGET == PLATFORM_MACOSX
    std::cout << "MacOSX" << std::endl;
#endif
    FBuffer buffer;
    buffer << 1 << "hello world" << true;
    int v;
    char buf[100] = {0};
    bool b;
    buffer >> v >> buf >> b;
    printf("v=%d,buf=%s, b = %d\n", v, buf, b);
    
    test();
    testbase64();
    testmd5();

    FThread thread(&foo);
    thread.start();
    thread.join();

    printf("--- ThreadGroup ---\r\n");
	FThreadGroup tg;
	FThread t1(&foo);
	tg.addThread(&t1);
	tg.createThread(&foo);
	tg.createThread(&foo);
	tg.startAll();
	tg.joinAll();

    FString fString;
    fString += _T(" nihao ");
    fString.Trim();
    fString << 456 << endl;
    const Fchar* pBuf = fString;
    std::cout << fString << std::endl;
    printf("fString = %s, length=%d,\n", pBuf, fString.GetLength());
    
    string str = FConvert::Convert<string>(v);
    int n = FConvert::Convert<int>("5");
    
    F_CONSOLE(ERROR) << F_FORMAT("n = %d, %s\n", n, FGetModulePath());
    F_CONSOLE(ERROR) << F_FORMAT("n = %d, %s\n", n, FGetModuleName());
    int *p = &n;
    F_CONSOLE(WARN) << (void*)p << endl;
    int counter = 0;
    clock_t t = clock();
    FCounter cnt;
    std::map<int, std::vector<int> > ids;
    for(int i=-128; i < 128; ++i)
    {
        for(int j=-128; j < 128; ++j)
        {
            for(int k=0;k<97;++k)
            {
                counter ++;
                cnt.Add();
                // printf("cnt = %d\n",cnt.GetCount());
                int dis = CalcDistance(i, j, k, i+j);
                if(ids.find(dis) != ids.end())
                {
                    ids[dis].push_back(k);
                }
                else
                {
                    std::vector<int> vv;
                    vv.push_back(k);
                    ids[dis] = vv;
                }

               FString s;
            }
        }
    }
    printf("clock = %lu\n", clock() - t);
    delete pT;

    FFile file;
    if(!file) F_CONSOLE(WARN) << F_FORMAT("file is invalid\n");
    int ret = file.Open("hello.txt", true);
    if(file) 
    {
        F_CONSOLE(WARN) << F_FORMAT("file is valid\n");
        //ret = file.Write("Hello File", 10);
        FAutoData data;
        ret = file.ReadAll(data);
        char buff[20] = {0};
        ret = file.Read(buff, 20);
        printf("write size = %d, size = %ld, offset = %ld, eof = %d\n", ret, file.GetSize(), file.GetOffset(), (int)(file.IsEOF()));
        ret = file.Read(buff+ret, 10);
        file.Close();
    }
    file.Open("/Users/lidengfeng/Documents/Workspace/Binary/Binary/bin/Debug/test.txt", true);
	if (file)
	{
		FAutoData bytes;
		long sz = file.GetSize();
		file.ReadAll(bytes);
		FBuffer br((const uint8 *)(char*)bytes, sz);
		int32 i32;
		int16 i16;
		int64 i64;
		char ib[1024] = { 0 };
		br >> i32 >> i16 >> i64 >> ib;
		F_CONSOLE(WARN) << i32 << "," << i16 << "," << i64 << "," << ib << endl;
	}
    FPlugin plugin("/Volumes/SHARED/WorkSpace/wLuaDemo/Demo/libwLua2.dylib");
    typedef int(*wlua_makecsindex)(void * , int);
    wlua_makecsindex pf = plugin.Get<wlua_makecsindex>("wlua_makecsindex");

    std::vector<std::string> textArr;
    FReadTextToArray("hello.txt", textArr);
    for(size_t i=0; i<textArr.size(); ++i)
    {
        std::cout << textArr[i].c_str() << std::endl;
    }
    FStringSplit(textArr, "adfiejfie\nsjdfiejf\nsifef你好\nasjfi", "\n");
    std::cout << "size = " << textArr.size() << std::endl;
    for(size_t i=0; i<textArr.size(); ++i)
    {
        std::cout << textArr[i].c_str() << std::endl;
    }
    return 0;
}
