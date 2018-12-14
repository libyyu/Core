#ifndef __FSTRING_HPP__
#define __FSTRING_HPP__
#pragma once
#include "FType.hpp"
#include <locale.h>
#include <assert.h>
#include <iostream>
#include <sstream> 
#include <stdarg.h>
#include <string.h>
_FStdBegin

class FString
{
public:
    enum { MAX_LOCAL_STRING_LEN = 63 };
    FString() : m_pstr(m_szBuffer) 
    {
        m_szBuffer[0] = '\0';
    }
	FString(const Fchar ch) : m_pstr(m_szBuffer)
    {
        m_szBuffer[0] = ch;
		m_szBuffer[1] = '\0';
    }
	FString(const Fchar* lpsz, int nLen = -1) : m_pstr(m_szBuffer)
    {
        assert(lpsz);
		m_szBuffer[0] = '\0';
		Assign(lpsz, nLen);
    }
    FString(const FString& src): m_pstr(m_szBuffer)
    {
        m_szBuffer[0] = '\0';
		Assign(src.m_pstr);
    }
    virtual ~FString()
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
		return (int)Fstrlen(m_pstr);
    }
    inline bool IsEmpty() const
    {
        return m_pstr[0] == '\0'; 
    }
    inline Fchar GetAt(int nIndex) const
    {
        return m_pstr[nIndex];
    }
	inline Fchar operator[] (int nIndex) const
    {
        return m_pstr[nIndex];
    }

	inline void SetAt(int nIndex, Fchar ch)
    {
        assert(nIndex>=0 && nIndex<GetLength());
		m_pstr[nIndex] = ch;
    }

	inline void Append(const Fchar* pstr)
    {
		int nNewLength = GetLength() + (int)Fstrlen(pstr);
		if( nNewLength >= MAX_LOCAL_STRING_LEN ) {
			if( m_pstr == m_szBuffer ) {
				m_pstr = static_cast<Fchar*>(malloc((nNewLength + 1) * sizeof(Fchar)));
				Fstrcpy(m_pstr, m_szBuffer);
				Fstrcat(m_pstr, pstr);
			}
			else {
				m_pstr = static_cast<Fchar*>(realloc(m_pstr, (nNewLength + 1) * sizeof(Fchar)));
				Fstrcat(m_pstr, pstr);
			}
		}
		else {
			if( m_pstr != m_szBuffer ) {
				free(m_pstr);
				m_pstr = m_szBuffer;
			}
			Fstrcat(m_szBuffer, pstr);
		}
    }

	inline void Assign(const Fchar* pstr, int cchMax = -1)
    {
        if( pstr == NULL ) pstr = _T("");
		cchMax = (cchMax < 0 ? (int)Fstrlen(pstr) : cchMax);
		if( cchMax < MAX_LOCAL_STRING_LEN ) {
			if( m_pstr != m_szBuffer ) {
				free(m_pstr);
				m_pstr = m_szBuffer;
			}
		}
		else if( cchMax > GetLength() || m_pstr == m_szBuffer ) {
			if( m_pstr == m_szBuffer ) m_pstr = NULL;
			m_pstr = static_cast<Fchar*>(realloc(m_pstr, (cchMax + 1) * sizeof(Fchar)));
		}
		Fstrncpy(m_pstr, pstr, cchMax);
		m_pstr[cchMax] = '\0';
    }
	inline int Compare(const Fchar* lpsz) const
	{ 
		return Fstrcmp(m_pstr, lpsz); 
	}

	inline int CompareNoCase(const Fchar* lpsz) const
	{ 
		return Fstrncasecmp(m_pstr, lpsz, Fstrlen(lpsz)); 
	}

    inline FString Left(int iLength) const
	{
		if( iLength < 0 ) iLength = 0;
		if( iLength > GetLength() ) iLength = GetLength();
		return FString(m_pstr, iLength);
	}

	inline FString Mid(int iPos, int iLength = -1) const
	{
		if( iLength < 0 ) iLength = GetLength() - iPos;
		if( iPos + iLength > GetLength() ) iLength = GetLength() - iPos;
		if( iLength <= 0 ) return FString();
		return FString(m_pstr + iPos, iLength);
	}

	inline FString Right(int iLength) const
	{
		int iPos = GetLength() - iLength;
		if( iPos < 0 ) {
			iPos = 0;
			iLength = GetLength();
		}
		return FString(m_pstr + iPos, iLength);
	}

    inline int Find(Fchar ch, int iPos = 0) const
	{
		assert(iPos>=0 && iPos<=GetLength());
		if( iPos != 0 && (iPos < 0 || iPos >= GetLength()) ) return -1;
		const Fchar* p = Fstrrchr(m_pstr + iPos, ch);
		if( p == NULL ) return -1;
		return (int)(p - m_pstr);
	}

	inline int Find(const Fchar* pstrSub, int iPos = 0) const
	{
		assert(iPos>=0 && iPos<=GetLength());
		if( iPos != 0 && (iPos < 0 || iPos > GetLength()) ) return -1;
		const Fchar* p = Fstrstr(m_pstr + iPos, pstrSub);
		if( p == NULL ) return -1;
		return (int)(p - m_pstr);
	}

	inline int ReverseFind(Fchar ch) const
	{
		const Fchar* p = Fstrrchr(m_pstr, ch);
		if( p == NULL ) return -1;
		return (int)(p - m_pstr);
	}

	inline int Replace(const Fchar* pstrFrom, const Fchar* pstrTo)
	{
		FString sTemp;
		int nCount = 0;
		int iPos = Find(pstrFrom);
		if( iPos < 0 ) return 0;
		int cchFrom = (int) Fstrlen(pstrFrom);
		int cchTo = (int) Fstrlen(pstrTo);
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

	inline int Format(const Fchar* pstrFormat, ...)
	{
		// Do ordinary printf replacements
		// NOTE: Documented max-length of wvsprintf() is 2048
		Fchar szBuffer[20480] = { 0 };
		va_list argList;
		va_start(argList, pstrFormat);

		int iRet = Fvsnprintf(szBuffer, sizeof(szBuffer) / sizeof(szBuffer[0]) - 2, pstrFormat, argList);
		
		va_end(argList);
		Assign(szBuffer);
		return iRet;
	}

	static FString Printf(const Fchar* pstrFormat, ...)
	{
		FString strOut;
		Fchar szBuffer[20480] = { 0 };
		va_list argList;
		va_start(argList, pstrFormat);

		Fvsnprintf(szBuffer, sizeof(szBuffer) / sizeof(szBuffer[0]) - 2, pstrFormat, argList);
		
		va_end(argList);
		strOut.Assign(szBuffer);
		return strOut;
	}

	inline void SplitToArray(std::vector<FString>& OutArr, const Fchar* pattern)
	{
		if (IsEmpty() || !pattern || pattern[0] == 0x0)
			return;

		int nCount = 0;
		FString temp;
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
		const Fchar* p = m_pstr;
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
		Fchar *p = m_pstr + len - 1;
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

	virtual operator Fchar*() const
    {
        return m_pstr; 
    }

	const FString& operator=(const Fchar ch)
    {
        Empty();
		m_szBuffer[0] = ch;
		m_szBuffer[1] = '\0';
		return *this;
    }

    const FString& operator=(const FString& src)
    {
        Assign(src);
		return *this;
    }
	const FString& operator=(const Fchar* lpStr)
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

    FString operator+(const FString& src) const
	{
		FString sTemp = *this;
		sTemp.Append(src);
		return sTemp;
	}
	FString operator+(const Fchar* lpStr) const
	{
		if ( lpStr )
		{
			FString sTemp = *this;
			sTemp.Append(lpStr);
			return sTemp;
		}

		return *this;
	}
	FString operator+(const Fchar ch) const
	{
		FString sTemp = *this;
		Fchar str[] = { ch, '\0' };
		sTemp.Append(str);
		return sTemp;
	}

    const FString& operator+=(const FString& src)
	{      
		Append(src);
		return *this;
	}
	const FString& operator+=(const Fchar* lpStr)
	{      
		if ( lpStr )
		{
			Append(lpStr);
		}
		
		return *this;
	}
	const FString& operator+=(const Fchar ch)
	{      
		Fchar str[] = { ch, '\0' };
		Append(str);
		return *this;
	}

	bool operator == (const Fchar* str) const { return (Compare(str) == 0); };
	bool operator != (const Fchar* str) const { return (Compare(str) != 0); };
	bool operator <= (const Fchar* str) const { return (Compare(str) <= 0); };
	bool operator <  (const Fchar* str) const { return (Compare(str) <  0); };
	bool operator >= (const Fchar* str) const { return (Compare(str) >= 0); };
	bool operator >  (const Fchar* str) const { return (Compare(str) >  0); };

	template<typename T>
    inline FString& operator<<(T v); // will generate link error
    inline FString& operator<<(int8 v);
    inline FString& operator<<(int16 v);
    inline FString& operator<<(int32 v);
    inline FString& operator<<(int64 v);
    inline FString& operator<<(uint8 v);
    inline FString& operator<<(uint16 v);
    inline FString& operator<<(uint32 v);
    inline FString& operator<<(uint64 v);
    inline FString& operator<<(bool v);
    inline FString& operator<<(float v);
    inline FString& operator<<(double v);
    inline FString& operator<<(const Fchar *str);
	inline FString& operator<<(Fchar v[]);
	inline FString& operator<<(Fstring& str);
    inline FString& operator<<(FString &v);
	inline FString& operator<< (FString& (*_f)(FString&));

	friend FString& endl(FString& v);
protected:
	template<typename T>
	inline void Write(const T &src);
private:
	Fchar* m_pstr;
	Fchar m_szBuffer[MAX_LOCAL_STRING_LEN + 1];
};

inline FString& operator<<(FString& str,const std::string &v)
{
	str << v;
    return str;
}
inline FString& operator<<(FString& str, const std::wstring &v)
{
	str << v;
	return str;
}

template<typename T>
inline void FString::Write(const T &src)
{
	Fstringstream str;
	str << src;
	Append(str.str().c_str());
}
FString& FString::operator<<(int8 v)
{
	Write<int8>(v);
	return (*this);
}
FString& FString::operator<<(int16 v)
{
	Write<int16>(v);
	return *this;
}
FString& FString::operator<<(int32 v)
{	
	Write<int32>(v);
	return *this;
}
FString& FString::operator<<(int64 v)
{
	Write<int64>(v);
	return *this;
}
FString& FString::operator<<(uint8 v)
{
	Write<uint8>(v);
	return *this;
}
FString& FString::operator<<(uint16 v)
{
	Write<uint16>(v);
	return *this;
}
FString& FString::operator<<(uint32 v)
{
	Write<uint32>(v);
	return *this;
}
FString& FString::operator<<(uint64 v)
{
	Write<uint64>(v);
	return *this;
}
FString& FString::operator<<(bool v)
{
	Write<bool>(v);
	return *this;
}
FString& FString::operator<<(float v)
{
	Write<float>(v);
	return *this;
}
FString& FString::operator<<(double v)
{
	Write<double>(v);
	return *this;
}
FString& FString::operator<<(const Fchar *str)
{
	Append(str);
	return *this;
}
FString& FString::operator<<(Fchar str[])
{
	Append(str);
	return *this;
}
FString& FString::operator<<(Fstring& str)
{
	(*this) << (str.c_str());
	return *this;
}
FString& FString::operator<<(FString &v)
{
	(*this) += v;
	return *this;
}
FString& FString::operator<< (FString& (*_f)(FString&))
{
	return _f(*this);
}
inline FString& endl(FString& v)
{
	v += _T('\n');
	return v;
}

_FStdEnd

#endif//__FSTRING_HPP__
