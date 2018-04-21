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

_RzUsing(std)
_RzUsing(RzStd)

int CalcDistance(int x, int z, int x2, int z2)
{
    int d = x - x2;
    int d2 = z - z2;
    return d*d + d2*d2;
}

int main(int argc, const char * argv[]) {
    // insert code here...
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
    
    CRzString rzString;
    rzString += " nihao ";
    rzString.Trim();
    rzString << 456 << endl;
    const char* pBuf = rzString;
    std::cout << rzString << std::endl;
    printf("rzString = %s, length=%d,\n", pBuf, rzString.GetLength());
    
    string str = CRzConvert::Convert<string>(v);
    int n = CRzConvert::Convert<int>("5");
    
    RZ_CONSOLE(ERROR) << RZ_FORMAT("n = %d, %s\n", n, RzGetModulePath());
    RZ_CONSOLE(ERROR) << RZ_FORMAT("n = %d, %s\n", n, RzGetModuleName());

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

    CRzFile file;
    if(!file) RZ_CONSOLE(WARN) << RZ_FORMAT("file is invalid\n");
    int ret = file.Open("hello.txt", true);
    if(file) RZ_CONSOLE(WARN) << RZ_FORMAT("file is valid\n");
    //ret = file.Write("Hello File", 10);
    char buff[20] = {0};
    ret = file.Read(buff, 20);
    printf("write size = %d, size = %ld, offset = %ld, eof = %d\n", ret, file.GetSize(), file.GetOffset(), (int)(file.IsEOF()));
    ret = file.Read(buff+ret, 10);
    file.Close();

    return 0;
}
