#ifndef __traits_hpp__
#define __traits_hpp__
#include <time.h>
#include <sstream>
#include <iostream>
#include "RzType.hpp"

_RzStdBegin
namespace trait
{
	template<typename T>
	struct type_traits
	{
		typedef T                      value_type;
		typedef T*                     pointer;
		typedef T&                     reference;
		typedef const T*               const_pointer;
		typedef const T&               const_reference;
	};

	template<typename T>
	struct type_traits<T*>
	{
		typedef T                      value_type;
		typedef T*                     pointer;
		typedef T&                     reference;
		typedef const T*               const_pointer;
		typedef const T&               const_reference;
	};

	template<typename T>
	struct type_traits<const T*>
	{
		typedef T                      value_type;
		typedef T*                     pointer;
		typedef T&                     reference;
		typedef const T*               const_pointer;
		typedef const T&               const_reference;
	};

	template<typename T>
	struct type_traits<T&>
	{
		typedef T                      value_type;
		typedef T*                     pointer;
		typedef T&                     reference;
		typedef const T*               const_pointer;
		typedef const T&               const_reference;
	};

	template<>
	struct type_traits<void>
	{
		typedef void                   value_type;
		typedef void*                  pointer;
		typedef const void*            const_pointer;
	};
}
_RzStdEnd

#endif//__traits_hpp__