//
//  main.cpp
//  ddddddd
//
//  Created by Dengfeng Li on 2017/11/26.
//  Copyright © 2017年 Dengfeng Li. All rights reserved.
//

#include <iostream>
#include "RzBuffer.hpp"
#include "RzString.hpp"
#include "RzObserver.hpp"
#include "RzConvert.hpp"
#include "RzConsole.hpp"
#include "RzFunc.hpp"
#include "RzFile.hpp"
#include "RzCounter.hpp"
#include "RzThread.hpp"
#include "RzMD5.hpp"
#include "RzBase64.hpp"
#include "RzMemTrack.hpp"
_RzUsing(std)
_RzUsing(RzStd)

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
	    printf("this is a thread function output %d\r\n", RzGetCurrentThreadId());
}

void test()
{
    RZ_CONSOLE_TRACE
}

void testbase64()
{
    const char* str = "helloworld";
    std::string code = RZ_BASE64_ENCODE(str);
    RZ_CONSOLE(DEBUG) << "base64:" << RZ_BASE64_ENCODE(str) << "," << RZ_BASE64_DECODE(code.c_str()) << endl;
}
void testmd5()
{
    char buff[200];
    RzMD5String("hello world", buff);
    RZ_CONSOLE(DEBUG) << buff << endl;
}

int main(int argc, const char * argv[]) {
    // insert code here...
    RZ_CONSOLE_TRACE
    int * pT = new int(8);
    std::cout << "Hello, World!\n";
#if PLATFORM_TARGET == PLATFORM_MACOSX
    std::cout << "MacOSX" << std::endl;
#endif
    CRzBuffer buffer;
    buffer << 1 << "hello world" << true;
    int v;
    char buf[100] = {0};
    bool b;
    buffer >> v >> buf >> b;
    printf("v=%d,buf=%s, b = %d\n", v, buf, b);
    
    test();
    testbase64();
    testmd5();

    CRzThread thread(&foo);
    thread.start();
    thread.join();

    printf("--- ThreadGroup ---\r\n");
	CRzThreadGroup tg;
	CRzThread t1(&foo);
	tg.addThread(&t1, false);
	tg.createThread(&foo);
	tg.createThread(&foo);
	tg.startAll();
	tg.joinAll();

    CRzString rzString;
    rzString += _T(" nihao ");
    rzString.Trim();
    rzString << 456 << endl;
    const Rzchar* pBuf = rzString;
    std::cout << rzString << std::endl;
    printf("rzString = %s, length=%d,\n", pBuf, rzString.GetLength());
    
    string str = CRzConvert::Convert<string>(v);
    int n = CRzConvert::Convert<int>("5");
    
    RZ_CONSOLE(ERROR) << RZ_FORMAT("n = %d, %s\n", n, RzGetModulePath());
    RZ_CONSOLE(ERROR) << RZ_FORMAT("n = %d, %s\n", n, RzGetModuleName());
    int *p = &n;
    RZ_CONSOLE(WARN) << (void*)p << endl;
    int counter = 0;
    clock_t t = clock();
    CRzCounter cnt;
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

               CRzString s;
            }
        }
    }
    printf("clock = %lu\n", clock() - t);
    delete pT;

    CRzFile file;
    if(!file) RZ_CONSOLE(WARN) << RZ_FORMAT("file is invalid\n");
    int ret = file.Open("hello.txt", true);
    if(file) 
    {
        RZ_CONSOLE(WARN) << RZ_FORMAT("file is valid\n");
        //ret = file.Write("Hello File", 10);
        RzAutoData data;
        ret = file.ReadAll(data);
        char buff[20] = {0};
        ret = file.Read(buff, 20);
        printf("write size = %d, size = %ld, offset = %ld, eof = %d\n", ret, file.GetSize(), file.GetOffset(), (int)(file.IsEOF()));
        ret = file.Read(buff+ret, 10);
        file.Close();
    }
    file.Open("/Users/lidengfeng/Documents/Workspace/Binary/Binary/bin/Debug/test.txt", true);
    RzAutoData bytes;
    long sz = file.GetSize();
    file.ReadAll(bytes);
    CRzBuffer br((const uint8 *)(char*)bytes, sz);
    int32 i32;
    int16 i16;
    int64 i64;
    char ib[1024] = {0};
    br >> i32 >> i16 >> i64 >> ib;
    RZ_CONSOLE(WARN) << i32 << "," << i16 << "," << i64 << "," << ib << endl;

    RZ_REPORT_MEMORY
    
    return 0;
}
