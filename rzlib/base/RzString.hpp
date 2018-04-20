#ifndef __RZSTRING_HPP__
#define __RZSTRING_HPP__
#pragma once
#include <string>
#include <locale.h>
#include <assert.h>
#include <iostream>
#include <sstream> 
#include "RzType.hpp"

_RzStdBegin

class CRzString
{
public:
    enum { MAX_LOCAL_STRING_LEN = 63 };
    CRzString() : m_pstr(m_szBuffer) 
    {
        m_szBuffer[0] = '\0';
    }
    CRzString(const char ch) : m_pstr(m_szBuffer)
    {
        m_szBuffer[0] = ch;
		m_szBuffer[1] = '\0';
    }
	CRzString(const char* lpsz, int nLen = -1) : m_pstr(m_szBuffer)
    {
        assert(lpsz);
		m_szBuffer[0] = '\0';
		Assign(lpsz, nLen);
    }
    CRzString(const CRzString& src): m_pstr(m_szBuffer)
    {
        m_szBuffer[0] = '\0';
		Assign(src.m_pstr);
    }
    virtual ~CRzString()
    {
        if( m_pstr != m_szBuffer && m_pstr != NULL) free(m_pstr);
    }
public:
    inline void Empty()
    {
        if( m_pstr != m_szBuffer ) free(m_pstr);
		m_pstr = m_szBuffer;
		m_szBuffer[0] = '\0';
    }
    inline int GetLength() const
    {
        return (int) strlen(m_pstr); 
    }
    inline bool IsEmpty() const
    {
        return m_pstr[0] == '\0'; 
    }
    inline char GetAt(int nIndex) const
    {
        return m_pstr[nIndex];
    }
    inline char operator[] (int nIndex) const
    {
        return m_pstr[nIndex];
    }

    inline void SetAt(int nIndex, char ch)
    {
        assert(nIndex>=0 && nIndex<GetLength());
		m_pstr[nIndex] = ch;
    }

    inline void Append(const char* pstr)
    {
        int nNewLength = GetLength() + (int) strlen(pstr);
		if( nNewLength >= MAX_LOCAL_STRING_LEN ) {
			if( m_pstr == m_szBuffer ) {
				m_pstr = static_cast<char*>(malloc((nNewLength + 1) * sizeof(char)));
				strcpy(m_pstr, m_szBuffer);
				strcat(m_pstr, pstr);
			}
			else {
				m_pstr = static_cast<char*>(realloc(m_pstr, (nNewLength + 1) * sizeof(char)));
				strcat(m_pstr, pstr);
			}
		}
		else {
			if( m_pstr != m_szBuffer ) {
				free(m_pstr);
				m_pstr = m_szBuffer;
			}
			strcat(m_szBuffer, pstr);
		}
    }

	inline void Assign(const char* pstr, int cchMax = -1)
    {
        if( pstr == NULL ) pstr = "";
		cchMax = (cchMax < 0 ? (int) strlen(pstr) : cchMax);
		if( cchMax < MAX_LOCAL_STRING_LEN ) {
			if( m_pstr != m_szBuffer ) {
				free(m_pstr);
				m_pstr = m_szBuffer;
			}
		}
		else if( cchMax > GetLength() || m_pstr == m_szBuffer ) {
			if( m_pstr == m_szBuffer ) m_pstr = NULL;
			m_pstr = static_cast<char*>(realloc(m_pstr, (cchMax + 1) * sizeof(char)));
		}
		strncpy(m_pstr, pstr, cchMax);
		m_pstr[cchMax] = '\0';
    }
    inline int Compare(const char* lpsz) const 
	{ 
		return strcmp(m_pstr, lpsz); 
	}

	inline int CompareNoCase(const char* lpsz) const 
	{ 
#if PLATFORM_TARGET == PLATFORM_WINDOWS
        return stricmp(m_pstr, lpsz); 
#else
		return strcasecmp(m_pstr, lpsz); 
#endif
	}

    inline CRzString Left(int iLength) const
	{
		if( iLength < 0 ) iLength = 0;
		if( iLength > GetLength() ) iLength = GetLength();
		return CRzString(m_pstr, iLength);
	}

	inline CRzString Mid(int iPos, int iLength = -1) const
	{
		if( iLength < 0 ) iLength = GetLength() - iPos;
		if( iPos + iLength > GetLength() ) iLength = GetLength() - iPos;
		if( iLength <= 0 ) return CRzString();
		return CRzString(m_pstr + iPos, iLength);
	}

	inline CRzString Right(int iLength) const
	{
		int iPos = GetLength() - iLength;
		if( iPos < 0 ) {
			iPos = 0;
			iLength = GetLength();
		}
		return CRzString(m_pstr + iPos, iLength);
	}

    inline int Find(char ch, int iPos = 0) const
	{
		assert(iPos>=0 && iPos<=GetLength());
		if( iPos != 0 && (iPos < 0 || iPos >= GetLength()) ) return -1;
		const char* p = strchr(m_pstr + iPos, ch);
		if( p == NULL ) return -1;
		return (int)(p - m_pstr);
	}

	inline int Find(const char* pstrSub, int iPos = 0) const
	{
		assert(iPos>=0 && iPos<=GetLength());
		if( iPos != 0 && (iPos < 0 || iPos > GetLength()) ) return -1;
		const char* p = strstr(m_pstr + iPos, pstrSub);
		if( p == NULL ) return -1;
		return (int)(p - m_pstr);
	}

	inline int ReverseFind(char ch) const
	{
		const char* p = strrchr(m_pstr, ch);
		if( p == NULL ) return -1;
		return (int)(p - m_pstr);
	}

    inline int Replace(const char* pstrFrom, const char* pstrTo)
	{
		CRzString sTemp;
		int nCount = 0;
		int iPos = Find(pstrFrom);
		if( iPos < 0 ) return 0;
		int cchFrom = (int) strlen(pstrFrom);
		int cchTo = (int) strlen(pstrTo);
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

    inline int Format(const char* pstrFormat, ...)
	{
		// Do ordinary printf replacements
		// NOTE: Documented max-length of wvsprintf() is 2048
		char szBuffer[2049] = { 0 };
		va_list argList;
		va_start(argList, pstrFormat);

		int iRet = vsnprintf(szBuffer, sizeof(szBuffer) / sizeof(szBuffer[0]) - 2, pstrFormat, argList);
		
		va_end(argList);
		Assign(szBuffer);
		return iRet;
	}

    inline void TrimLeft()
    {
        const char* p = m_pstr;
        while (*p != '\0')
        {
            if(*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
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
        char *p = m_pstr + len - 1;
        while (p >= m_pstr)
        {
            if(*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
            {
                *p = '\0';
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

	virtual operator char*() const
    {
        return m_pstr; 
    }

    const CRzString& operator=(const char ch)
    {
        Empty();
		m_szBuffer[0] = ch;
		m_szBuffer[1] = '\0';
		return *this;
    }

    const CRzString& operator=(const CRzString& src)
    {
        Assign(src);
		return *this;
    }
    const CRzString& operator=(const char* lpStr)
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

    CRzString operator+(const CRzString& src) const
	{
		CRzString sTemp = *this;
		sTemp.Append(src);
		return sTemp;
	}
    CRzString operator+(const char* lpStr) const
	{
		if ( lpStr )
		{
			CRzString sTemp = *this;
			sTemp.Append(lpStr);
			return sTemp;
		}

		return *this;
	}
    CRzString operator+(const char ch) const
	{
		CRzString sTemp = *this;
        char str[] = { ch, '\0' };
		sTemp.Append(str);
		return sTemp;
	}

    const CRzString& operator+=(const CRzString& src)
	{      
		Append(src);
		return *this;
	}
    const CRzString& operator+=(const char* lpStr)
	{      
		if ( lpStr )
		{
			Append(lpStr);
		}
		
		return *this;
	}
    const CRzString& operator+=(const char ch)
	{      
		char str[] = { ch, '\0' };
		Append(str);
		return *this;
	}

    bool operator == (const char* str) const { return (Compare(str) == 0); };
	bool operator != (const char* str) const { return (Compare(str) != 0); };
	bool operator <= (const char* str) const { return (Compare(str) <= 0); };
	bool operator <  (const char* str) const { return (Compare(str) <  0); };
	bool operator >= (const char* str) const { return (Compare(str) >= 0); };
	bool operator >  (const char* str) const { return (Compare(str) >  0); };

	template<typename T>
    inline CRzString& operator<<(T v); // will generate link error
    inline CRzString& operator<<(int8 v);
    inline CRzString& operator<<(int16 v);
    inline CRzString& operator<<(int32 v);
    inline CRzString& operator<<(int64 v);
    inline CRzString& operator<<(uint8 v);
    inline CRzString& operator<<(uint16 v);
    inline CRzString& operator<<(uint32 v);
    inline CRzString& operator<<(uint64 v);
    inline CRzString& operator<<(bool v);
    inline CRzString& operator<<(float v);
    inline CRzString& operator<<(double v);
    inline CRzString& operator<<(const char *str);
	inline CRzString& operator<<(std::string& str);
    inline CRzString& operator<<(CRzString &v);
	inline CRzString& operator<< (CRzString& (*_f)(CRzString&));

	friend CRzString& endl(CRzString& v);
protected:
	template<typename T>
	inline void Write(const T &src);
private:
    char* m_pstr;
    char m_szBuffer[MAX_LOCAL_STRING_LEN + 1];
};

inline CRzString& operator<<(CRzString& str,const std::string &v)
{
    str.Append(v.c_str());
    return str;
}

template<typename T>
inline void CRzString::Write(const T &src)
{
	std::stringstream str;	
	str << src;
	Append(str.str().c_str());
}
CRzString& CRzString::operator<<(int8 v)
{
	Write<int8>(v);
	return (*this);
}
CRzString& CRzString::operator<<(int16 v)
{
	Write<int16>(v);
	return *this;
}
CRzString& CRzString::operator<<(int32 v)
{	
	Write<int32>(v);
	return *this;
}
CRzString& CRzString::operator<<(int64 v)
{
	Write<int64>(v);
	return *this;
}
CRzString& CRzString::operator<<(uint8 v)
{
	Write<uint8>(v);
	return *this;
}
CRzString& CRzString::operator<<(uint16 v)
{
	Write<uint16>(v);
	return *this;
}
CRzString& CRzString::operator<<(uint32 v)
{
	Write<uint32>(v);
	return *this;
}
CRzString& CRzString::operator<<(uint64 v)
{
	Write<uint64>(v);
	return *this;
}
CRzString& CRzString::operator<<(bool v)
{
	Write<bool>(v);
	return *this;
}
CRzString& CRzString::operator<<(float v)
{
	Write<float>(v);
	return *this;
}
CRzString& CRzString::operator<<(double v)
{
	Write<double>(v);
	return *this;
}
CRzString& CRzString::operator<<(const char *str)
{
	Append(str);
	return *this;
}
CRzString& CRzString::operator<<(std::string& str)
{
	Append(str.c_str());
	return *this;
}
CRzString& CRzString::operator<<(CRzString &v)
{
	(*this) += v;
	return *this;
}
CRzString& CRzString::operator<< (CRzString& (*_f)(CRzString&))
{
	return _f(*this);
}
CRzString& endl(CRzString& v)
{
	v += '\n';
	return v;
}

_RzStdEnd

#endif//__RZSTRING_HPP__
