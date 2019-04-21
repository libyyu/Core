#ifndef __FWSTRING_HPP__
#define __FWSTRING_HPP__
#pragma once
#include "FType.hpp"
#include <locale.h>
#include <assert.h>
#include <iostream>
#include <sstream> 
#include <stdarg.h>
#include <string.h>
_FStdBegin

class FWString
{
public:
    enum { MAX_LOCAL_STRING_LEN = 63 };
    FWString() : m_pstr(m_szBuffer) 
    {
        m_szBuffer[0] = L'\0';
    }
	FWString(const wchar_t ch) : m_pstr(m_szBuffer)
    {
        m_szBuffer[0] = ch;
		m_szBuffer[1] = L'\0';
    }
	FWString(const wchar_t* lpsz, int nLen = -1) : m_pstr(m_szBuffer)
    {
        assert(lpsz);
		m_szBuffer[0] = L'\0';
		Assign(lpsz, nLen);
    }
    FWString(const FWString& src): m_pstr(m_szBuffer)
    {
        m_szBuffer[0] = L'\0';
		Assign(src.m_pstr);
    }
    virtual ~FWString()
    {
        if( m_pstr != m_szBuffer && m_pstr != NULL) free(m_pstr);
    }
public:
    inline void Empty()
    {
        if( m_pstr != m_szBuffer ) free(m_pstr);
		m_pstr = m_szBuffer;
		m_szBuffer[0] = L'\0';
    }
    inline size_t GetLength() const
    {
		return wcslen(m_pstr);
    }
    inline bool IsEmpty() const
    {
        return m_pstr[0] == L'\0'; 
    }
    inline wchar_t GetAt(int nIndex) const
    {
        return m_pstr[nIndex];
    }
	inline wchar_t operator[] (int nIndex) const
    {
        return m_pstr[nIndex];
    }

	inline void SetAt(int nIndex, wchar_t ch)
    {
        assert(nIndex>=0 && nIndex<GetLength());
		m_pstr[nIndex] = ch;
    }

	inline void Append(const wchar_t* pstr)
    {
        assert(pstr);
		size_t nNewLength = GetLength() + wcslen(pstr);
		if( nNewLength >= MAX_LOCAL_STRING_LEN ) {
			if( m_pstr == m_szBuffer ) {
				m_pstr = static_cast<wchar_t*>(malloc((nNewLength + 1) * sizeof(wchar_t)));
				wcscpy(m_pstr, m_szBuffer);
				wcscat(m_pstr, pstr);
			}
			else {
				m_pstr = static_cast<wchar_t*>(realloc(m_pstr, (nNewLength + 1) * sizeof(wchar_t)));
				wcscat(m_pstr, pstr);
			}
		}
		else {
			if( m_pstr != m_szBuffer ) {
				free(m_pstr);
				m_pstr = m_szBuffer;
			}
			wcscat(m_szBuffer, pstr);
		}
    }

	inline void Assign(const wchar_t* pstr, int cchMax = -1)
    {
        if( pstr == NULL ) pstr = L"";
		cchMax = (cchMax < 0 ? wcslen(pstr) : cchMax);
		if( cchMax < MAX_LOCAL_STRING_LEN ) {
			if( m_pstr != m_szBuffer ) {
				free(m_pstr);
				m_pstr = m_szBuffer;
			}
		}
		else if( cchMax > GetLength() || m_pstr == m_szBuffer ) {
			if( m_pstr == m_szBuffer ) m_pstr = NULL;
			m_pstr = static_cast<wchar_t*>(realloc(m_pstr, (cchMax + 1) * sizeof(wchar_t)));
		}
		wcsncpy(m_pstr, pstr, cchMax);
		m_pstr[cchMax] = '\0';
    }
	inline int Compare(const wchar_t* lpsz) const
	{ 
		return wcscmp(m_pstr, lpsz); 
	}

	inline int CompareNoCase(const wchar_t* lpsz) const
	{ 
#if FLIB_COMPILER_MSVC || FLIB_COMPILER_CYGWIN
		return _wcsnicmp(m_pstr, lpsz, wcslen(lpsz)); 
#else
		return wcsncasecmp(m_pstr, lpsz, wcslen(lpsz)); 
#endif
	}

    inline FWString Left(int iLength) const
	{
		if( iLength < 0 ) iLength = 0;
		if( iLength > GetLength() ) iLength = GetLength();
		return FWString(m_pstr, iLength);
	}

	inline FWString Mid(int iPos, int iLength = -1) const
	{
		if( iLength < 0 ) iLength = GetLength() - iPos;
		if( iPos + iLength > GetLength() ) iLength = GetLength() - iPos;
		if( iLength <= 0 ) return FWString();
		return FWString(m_pstr + iPos, iLength);
	}

	inline FWString Right(int iLength) const
	{
		int iPos = GetLength() - iLength;
		if( iPos < 0 ) {
			iPos = 0;
			iLength = GetLength();
		}
		return FWString(m_pstr + iPos, iLength);
	}

    inline int Find(wchar_t ch, int iPos = 0) const
	{
		assert(iPos>=0 && iPos<=GetLength());
		if( iPos != 0 && (iPos < 0 || iPos >= GetLength()) ) return -1;
		const wchar_t* p = wcsrchr(m_pstr + iPos, ch);
		if( p == NULL ) return -1;
		return (int)(p - m_pstr);
	}

	inline int Find(const wchar_t* pstrSub, int iPos = 0) const
	{
		assert(iPos>=0 && iPos<=GetLength());
		if( iPos != 0 && (iPos < 0 || iPos > GetLength()) ) return -1;
		const wchar_t* p = wcsstr(m_pstr + iPos, pstrSub);
		if( p == NULL ) return -1;
		return (int)(p - m_pstr);
	}

	inline int ReverseFind(wchar_t ch) const
	{
		const wchar_t* p = wcsrchr(m_pstr, ch);
		if( p == NULL ) return -1;
		return (int)(p - m_pstr);
	}

	inline int Replace(const wchar_t* pstrFrom, const wchar_t* pstrTo)
	{
		FWString sTemp;
		int nCount = 0;
		int iPos = Find(pstrFrom);
		if( iPos < 0 ) return 0;
		size_t cchFrom = wcslen(pstrFrom);
		size_t cchTo = wcslen(pstrTo);
		while( iPos >= 0 ) {
			sTemp = Left(iPos);
			sTemp += pstrTo;
			sTemp += Mid(iPos + cchFrom);
			Assign(sTemp);
			iPos = Find(pstrFrom, iPos + cchTo);
			nCount++;
		}
		return nCount;
	}

	inline int Format(const wchar_t* pstrFormat, ...)
	{
		// Do ordinary printf replacements
		// NOTE: Documented max-length of wvsprintf() is 2048
		wchar_t szBuffer[20480] = { 0 };
		va_list argList;
		va_start(argList, pstrFormat);
#if FLIB_COMPILER_MSVC || FLIB_COMPILER_CYGWIN
        int iRet = _vsnwprintf_s(szBuffer, sizeof(szBuffer) / sizeof(szBuffer[0]) - 2, pstrFormat, argList);
#else
		int iRet = vswprintf(szBuffer, sizeof(szBuffer) / sizeof(szBuffer[0]) - 2, pstrFormat, argList);
#endif		
		va_end(argList);
		Assign(szBuffer);
		return iRet;
	}

	static FWString Printf(const wchar_t* pstrFormat, ...)
	{
		FWString strOut;
		wchar_t szBuffer[20480] = { 0 };
		va_list argList;
		va_start(argList, pstrFormat);
#if FLIB_COMPILER_MSVC || FLIB_COMPILER_CYGWIN
        _vsnwprintf_s(szBuffer, sizeof(szBuffer) / sizeof(szBuffer[0]) - 2, pstrFormat, argList);
#else
		vswprintf(szBuffer, sizeof(szBuffer) / sizeof(szBuffer[0]) - 2, pstrFormat, argList);
#endif
		va_end(argList);
		strOut.Assign(szBuffer);
		return strOut;
	}

	inline void SplitToArray(std::vector<FWString>& OutArr, const wchar_t* pattern)
	{
		if (IsEmpty() || !pattern || pattern[0] == 0x0)
			return;

		int nCount = 0;
		FWString temp;
		size_t pos = 0, offset = 0;

		// 分割第1~n-1个
		while((pos = Find(pattern, offset)) != -1)
		{
			temp = Mid(offset, pos - offset);
			if (temp.GetLength() > 0)
			{
				OutArr.push_back(temp);
			}
			offset = pos + 1;
		}

		// 分割第n个
		temp = Mid(offset, GetLength() - offset);
		if (temp.GetLength() > 0)
		{
			OutArr.push_back(temp);
		}
	}

    inline void TrimLeft()
    {
		const wchar_t* p = m_pstr;
        while (*p != L'\0')
        {
            if(*p == L' ' || *p == L'\t' || *p == L'\r' || *p == L'\n')
            {
                ++p;
            }
            else
            {
                break;
            }
        }
        Assign(p);
    }
    
    inline void TrimRight()
    {
        int len = GetLength();
		wchar_t *p = m_pstr + len - 1;
        while (p >= m_pstr)
        {
            if(*p == L' ' || *p == L'\t' || *p == L'\r' || *p == L'\n')
            {
                *p = L'\0';
                --p;
            }
            else
            {
                break;
            }
        }
    }
    
    inline void Trim()
    {
        TrimLeft();
        TrimRight();
    }

	virtual operator wchar_t*() const
    {
        return m_pstr; 
    }

	const FWString& operator=(const wchar_t ch)
    {
        Empty();
		m_szBuffer[0] = ch;
		m_szBuffer[1] = L'\0';
		return *this;
    }

    const FWString& operator=(const FWString& src)
    {
        Assign(src);
		return *this;
    }
	const FWString& operator=(const wchar_t* lpStr)
    {
        if ( lpStr )
		{
			Assign(lpStr);
		}
		else
		{
			Empty();
		}
		return *this;
    }

    FWString operator+(const FWString& src) const
	{
		FWString sTemp = *this;
		sTemp.Append(src);
		return sTemp;
	}
	FWString operator+(const wchar_t* lpStr) const
	{
		if ( lpStr )
		{
			FWString sTemp = *this;
			sTemp.Append(lpStr);
			return sTemp;
		}

		return *this;
	}
	FWString operator+(const wchar_t ch) const
	{
		FWString sTemp = *this;
		wchar_t str[] = { ch, L'\0' };
		sTemp.Append(str);
		return sTemp;
	}

    const FWString& operator+=(const FWString& src)
	{      
		Append(src);
		return *this;
	}
	const FWString& operator+=(const wchar_t* lpStr)
	{      
		if ( lpStr )
		{
			Append(lpStr);
		}
		
		return *this;
	}
	const FWString& operator+=(const wchar_t ch)
	{      
		wchar_t str[] = { ch, L'\0' };
		Append(str);
		return *this;
	}

	bool operator == (const wchar_t* str) const { return (Compare(str) == 0); };
	bool operator != (const wchar_t* str) const { return (Compare(str) != 0); };
	bool operator <= (const wchar_t* str) const { return (Compare(str) <= 0); };
	bool operator <  (const wchar_t* str) const { return (Compare(str) <  0); };
	bool operator >= (const wchar_t* str) const { return (Compare(str) >= 0); };
	bool operator >  (const wchar_t* str) const { return (Compare(str) >  0); };

	template<typename T>
    inline FWString& operator<<(T v); // will generate link error
    inline FWString& operator<<(int8 v);
    inline FWString& operator<<(int16 v);
    inline FWString& operator<<(int32 v);
    inline FWString& operator<<(int64 v);
    inline FWString& operator<<(uint8 v);
    inline FWString& operator<<(uint16 v);
    inline FWString& operator<<(uint32 v);
    inline FWString& operator<<(uint64 v);
    inline FWString& operator<<(bool v);
    inline FWString& operator<<(float v);
    inline FWString& operator<<(double v);
    inline FWString& operator<<(const wchar_t *str);
	inline FWString& operator<<(wchar_t v[]);
	inline FWString& operator<<(std::wstring& str);
    inline FWString& operator<<(FWString &v);
	inline FWString& operator<< (FWString& (*_f)(FWString&));

	friend FWString& endl(FWString& v);
protected:
	template<typename T>
	inline void Write(const T &src);
private:
	wchar_t* m_pstr;
	wchar_t m_szBuffer[MAX_LOCAL_STRING_LEN + 1];
};

inline FWString& operator<<(FWString& str,const std::wstring &v)
{
	str << v;
    return str;
}

template<typename T>
inline void FWString::Write(const T &src)
{
	std::stringstream str;
	str << src;
	Append(str.str().c_str());
}
FWString& FWString::operator<<(int8 v)
{
	Write<int8>(v);
	return (*this);
}
FWString& FWString::operator<<(int16 v)
{
	Write<int16>(v);
	return *this;
}
FWString& FWString::operator<<(int32 v)
{	
	Write<int32>(v);
	return *this;
}
FWString& FWString::operator<<(int64 v)
{
	Write<int64>(v);
	return *this;
}
FWString& FWString::operator<<(uint8 v)
{
	Write<uint8>(v);
	return *this;
}
FWString& FWString::operator<<(uint16 v)
{
	Write<uint16>(v);
	return *this;
}
FWString& FWString::operator<<(uint32 v)
{
	Write<uint32>(v);
	return *this;
}
FWString& FWString::operator<<(uint64 v)
{
	Write<uint64>(v);
	return *this;
}
FWString& FWString::operator<<(bool v)
{
	Write<bool>(v);
	return *this;
}
FWString& FWString::operator<<(float v)
{
	Write<float>(v);
	return *this;
}
FWString& FWString::operator<<(double v)
{
	Write<double>(v);
	return *this;
}
FWString& FWString::operator<<(const wchar_t *str)
{
	Append(str);
	return *this;
}
FWString& FWString::operator<<(wchar_t str[])
{
	Append(str);
	return *this;
}
FWString& FWString::operator<<(std::wstring& str)
{
	(*this) << (str.c_str());
	return *this;
}
FWString& FWString::operator<<(FWString &v)
{
	(*this) += v;
	return *this;
}
FWString& FWString::operator<< (FWString& (*_f)(FWString&))
{
	return _f(*this);
}
inline FWString& endl(FWString& v)
{
	v += _T('\n');
	return v;
}

_FStdEnd

#endif//__FWSTRING_HPP__
