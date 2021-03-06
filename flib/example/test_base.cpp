//
//  main.cpp
//  ddddddd
//
//  Created by Dengfeng Li on 2017/11/26.
//  Copyright © 2017年 Dengfeng Li. All rights reserved.
//
#include "flib.h"
_FUsing(std)
_FUsing(FStd)
static FAutoFile fGlobalLog("log.txt");
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
    F_CONSOLE(DEBUG) << "base64:" << code.c_str() << "," << F_BASE64_DECODE(code.c_str()).c_str() << endl;
}
void testmd5()
{
    F_CONSOLE_TRACE
    F_LOGFILE_TRACE(fGlobalLog)
    char buff[200];
    FMD5String("hello world", buff);
    F_CONSOLE(DEBUG) << "md5:" << buff << endl;
    F_LOGFILE(DEBUG, fGlobalLog) << "md5:" << buff << endl;
    F_LOGFILE(DEBUG, fGlobalLog) << "test md5" << endl;
}

void test_base()
{
    F_CONSOLE_TRACE
    int * pT = new int(8);
    std::cout << "Hello, World!\n";
#if FLIB_COMPILER_MACOSX
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
    
    std::string str = FConvert::Convert<std::string>(v);
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
    //delete pT;

    FFile file;
    if(!file) F_CONSOLE(WARN) << F_FORMAT("file is invalid\n");
    int ret = file.Open("hello.txt", true);
    if(file) 
    {
        F_CONSOLE(WARN) << F_FORMAT("file is valid\n");
        //ret = file.Write("Hello File", 10);
        char buff[20] = {0};
        ret = file.Read(buff, 20);
        printf("write size = %d, size = %ld, offset = %ld, eof = %d\n", ret, file.GetSize(), file.GetOffset(), (int)(file.IsEOF()));
        ret = file.Read(buff+ret, 10);
        file.Close();
    }
    file.Open("/Users/lidengfeng/Documents/Workspace/Binary/Binary/bin/Debug/test.txt", true);
	if (file)
	{
        long nSize = file.GetSize();
        char* bytes = new char[nSize+1];
		long sz = file.GetSize();
		file.ReadAll(bytes);
        bytes[nSize] = 0x0;
		FBuffer br((const uint8 *)(char*)bytes, sz);
		int32 i32;
		int16 i16;
		int64 i64;
		char ib[1024] = { 0 };
		br >> i32 >> i16 >> i64 >> ib;
		F_CONSOLE(WARN) << i32 << "," << i16 << "," << i64 << "," << ib << endl;
        delete []bytes;
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

    std::vector<FString> strArray;
    FString fs("ajfdiej\najsifje\nasfe");
    fs.SplitToArray(strArray, "\n");
    for(size_t i=0; i<strArray.size(); ++i)
    {
        std::cout << strArray[i] << std::endl;
    }
}

void testv(FValue* value)
{
    FStd::string v("helloA");
    value->set(v.c_str());
}
void testv2(FValue* value)
{
    enum eT
    {
        A,
        B,
    };
    flib_enum_t e((int)(eT::A), "eT::A");
    value->set(&e);
}

void testvalue()
{
    F_CONSOLE_TRACE
    FValue value;
    testv(&value);
    std::cout << value.get_string() << std::endl;
    FValue* v = value.clone();
    std::cout << value.get_string() << std::endl;
    std::cout << v->get_string() << std::endl;
    testv2(v);
    delete v;
}

void test_csv()
{
    FDataTable dt;
    dt.LoadFromFile("/Users/lidengfeng/Library/Containers/com.tencent.xinWeChat/Data/Library/Application Support/com.tencent.xinWeChat/2.0b4.0.9/3aba8eb0d9748a6d3928292c8632d885/Message/MessageTemp/51b4403af5647e341ec8bbbebeadb0e7/File/localized.csv");

    std::cout << dt.ToString().c_str() << std::endl;
    FILE* fp = fopen("test.csv", "wb");
    fwrite(dt.ToString().c_str(), 1, dt.ToString().size(), fp);
    fclose(fp);
}

int main(int argc, const char * argv[]) {
    // insert code here...
    F_CONSOLE_TRACE
    F_LOGFILE_TRACE(fGlobalLog)
    F_LOGFILE(DEBUG, fGlobalLog) << "LOGFILE" << ", Test" << endl;

    test_base();

    testbase64();
    testmd5();

    testvalue();

    test_csv();

    FStd::TestStream stream;

    stream.stream() << "hello\n";

    return 0;
}
