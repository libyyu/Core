#include <boost/regex.hpp>  
#include <boost/lambda/lambda.hpp>  
#include <iostream>  
#include <string> 
#include <iterator>  
#include <algorithm>  
int main()  
{  
    std::chrono::nanoseconds timeout;
    std::string line;  
    boost::regex pat( "^Subject: (Re: |Aw: )*(.*)" );  
  
    while (std::cin)  
    {  
        std::getline(std::cin, line);  
        boost::smatch matches;  
        if (boost::regex_match(line, matches, pat))  
            std::cout << matches[2] << std::endl;  
    } 
    using namespace boost::lambda;  
    typedef std::istream_iterator<int> in;  
  
    std::for_each(  
        in(std::cin), in(), std::cout << (_1 * 3) << " " );   
} 