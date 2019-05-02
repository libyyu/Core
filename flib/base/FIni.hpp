#ifndef __FINI_HPP__
#define __FINI_HPP__
#pragma once
#include "FType.hpp"
#include <map> 
#include <fstream>
#include <sstream>
#include <iostream>
#include <functional>
#include <memory>
_FStdBegin

class FIni
{
public:
	typedef char														chartype;
	typedef std::string                                                 stringtype;
	typedef std::ifstream                                               ifstreamtype;
	typedef std::ofstream                                               ofstreamtype;

	struct FINI_KEYVALUE
	{
		bool			hasComment;	//	true, Comment
		stringtype		strComment;	//	comment
		stringtype		strKey;		//	Key name
		stringtype		strValue;	//	Value string

		stringtype ToString()
		{
			stringtype result;
			if (hasComment)
			{
				result += ";";
				result += strComment;
				result += "\n";
			}
			result += strKey + "=" + strValue;
			return result;
		}
	};

	struct FINI_SECTION
	{
		bool					hasComment;
		stringtype				strComment;	
		stringtype				strSession;
		std::vector<FINI_KEYVALUE*>	values;	//	Keys
		
		stringtype ToString()
		{
			stringtype result;
			if (hasComment)
			{
				result += ";";
				result += strComment;
				result += "\n";
			}
			result += "[" + strSession + "]";
			result += "\n";

			size_t cursor = 0;
			for (std::vector<FINI_KEYVALUE*>::const_iterator it = values.begin(); it != values.end(); ++it, ++cursor)
			{
				result += (*it)->ToString();
				/*if(cursor < values.size()-1) */result += "\n";
			}

			return result;
		}
	};

	typedef std::map<stringtype, FINI_SECTION*, std::less<stringtype> >          FINI_SectionMap;
private:
    FINI_SectionMap _SessionArr;
public:
	FIni(){};
	~FIni() { Clear(); };
	inline void Clear()
	{
		for (FINI_SectionMap::iterator itor = _SessionArr.begin(); itor != _SessionArr.end(); ++itor)
		{
			FINI_SECTION * pKeyMap = (itor->second);
			for (std::vector<FINI_KEYVALUE*>::iterator it = pKeyMap->values.begin(); it != pKeyMap->values.end(); ++it)
			{
				FINI_KEYVALUE* pKV = *it;
				delete pKV;
			}
			pKeyMap->values.clear();
			delete pKeyMap;
		}
		_SessionArr.clear();
	}
public:
	inline FINI_SECTION* operator[] (const char* psection)
	{
		return GetSession(psection);
	}
	inline FINI_SECTION* operator[](const stringtype& section)
	{
		return (*this)[section.c_str()];
	}

	inline bool OpenIni(const char* pFile)
	{
		assert(pFile);
		if(NULL == pFile)
			return false;

		ifstreamtype ifs(pFile);
		if(!ifs.is_open())
			return false;

		Clear();

		std::vector<stringtype> filebuf;
		std::vector<stringtype>::const_iterator itor;
		while(!ifs.eof())
		{
			stringtype buf;	
			getline(ifs,buf);
			filebuf.push_back(buf);
		}
		stringtype curSection = "";
		stringtype curComment = "";
		bool bComment = false;
		for(itor=filebuf.begin(); itor!=filebuf.end(); ++itor)
		{
			curSection = DoSwitchALine(*itor, curSection, curComment, bComment);
		}	
		return true;
	}
	inline bool OpenFromString(const char* content)
	{
		assert(content);
		if (!content) return false;
		Clear();
		const chartype* p = content;
		const chartype* prev = content;
		std::vector<stringtype> filebuf;
		std::vector<stringtype>::const_iterator itor;
		while (*p)
		{
			if (*p == '\n')
			{
				stringtype buf(prev, p - prev);
				p++;
				prev = p;
				filebuf.push_back(buf);
			}
			p++;
		}
		stringtype curSection = "";
		stringtype curComment = "";
		bool bComment = false;
		for (itor = filebuf.begin(); itor != filebuf.end(); ++itor)
		{
			curSection = DoSwitchALine(*itor, curSection, curComment, bComment);
		}
		return true;
	}
	inline bool SaveIni(const char* pFile)
	{
		assert(pFile);
		if (NULL == pFile)
			return false;

		ofstreamtype ofs(pFile);
		if (!ofs.is_open())
			return false;

		stringtype content = ToString();
		ofs << content.c_str();

		ofs.flush();
		ofs.close();
		return true;
	}
	
	inline stringtype GetStr(const chartype* SectionStr,const chartype* KeyStr, const chartype* DefaultStr = "") const
  {
		stringtype value;
		if (!GetString(SectionStr, KeyStr, value) || value.empty())
			value = DefaultStr;

		return value;
  }

	inline int GetInt(const chartype* pSection, const chartype* pKey, int nDefault = 0) const
	{
		stringtype value;
		if (!GetString(pSection, pKey, value) || value.empty())
			return nDefault;

		return atoi(value.c_str());
	}
	inline bool GetBool(const chartype* pSection, const chartype* pKey, bool bDefault = false)  const
	{
		return GetInt(pSection, pKey, bDefault ? 1 : 0) != 0 ? true : false;
	}
	inline float GetFloat(const chartype* pSection, const chartype* pKey, float fDefault = 0.0f)  const
	{
		stringtype value;
		if (!GetString(pSection, pKey, value) || value.empty())
			return fDefault;

		return (float)atof(value.c_str());
	}

	inline void AddStr(const chartype* pSection, const chartype* pKey, const chartype* pValue, const chartype* strSessionComment = NULL, const chartype* strValueComment = NULL)
	{
		AddSection(pSection, strSessionComment);
		AddKey(pSection, pKey, pValue, strValueComment);
	}
	inline void AddInt(const chartype* pSection, const chartype* pKey, int iValue, const chartype* strSessionComment = NULL, const chartype* strValueComment = NULL)
	{
		AddSection(pSection, strSessionComment);
		stringtype str = std::to_string(iValue);
		AddKey(pSection, pKey, str.c_str(), strValueComment);
	}
	inline void AddFloat(const chartype* pSection, const chartype* pKey, float fValue, const chartype* strSessionComment = NULL, const chartype* strValueComment = NULL)
	{
		stringtype str = std::to_string(fValue);
		AddSection(pSection, strSessionComment);
		AddKey(pSection, pKey, str.c_str(), strValueComment);
	}
	inline void AddBool(const chartype* pSection, const chartype* pKey, bool bValue, const chartype* strSessionComment = NULL, const chartype* strValueComment = NULL)
	{
		stringtype sValue = bValue ? "1" : "0";
		AddSection(pSection, strSessionComment);
		AddKey(pSection, pKey, sValue.c_str(), strValueComment);
	}

	inline FINI_SECTION* GetSession(const chartype* SectionStr)
	{
		return FindSession(SectionStr);
	}
	inline FINI_SECTION* CreateIfNoSession(const chartype* SectionStr, const chartype* strComment = NULL)
	{
		return AddSection(SectionStr, strComment);
	}

	stringtype ToString()
	{
		size_t cursor = 0;
		std::stringstream str;
		for (FINI_SectionMap::iterator itor = _SessionArr.begin(); itor != _SessionArr.end(); ++itor, ++cursor)
		{
			FINI_SECTION * pKeyMap = (itor->second);
			str << pKeyMap->ToString();
			if(cursor < _SessionArr.size()-1) str << std::endl;
		}
		
		return str.str();
	}

#ifdef _DEBUG
    inline void PrintIni()
    { 
		std::cout << ToString().c_str() << std::endl;
    }
#endif

 private:  
   inline stringtype DoSwitchALine(const stringtype &aLineStr, stringtype &curSection, stringtype& curComment, bool& bComment)
   {
	   if(aLineStr.empty()) return curSection;  //空行，不考虑
	   if (';' == aLineStr.at(0)) //注释
	   {
		   bComment = true;
		   curComment = aLineStr.substr(1);
		   return curSection;
	   }

	   stringtype::size_type n;	
	   if('[' == aLineStr.at(0)) //section开始
	   {
			n = aLineStr.find(']', 1);
			curSection = aLineStr.substr(1, n-1);
			AddSection(curSection.c_str(), bComment ? curComment.c_str() : NULL);
			bComment = false;
			curComment = "";
			return curSection;		
	   }
 
	   stringtype strKey;
	   stringtype strVaule;
	   n = aLineStr.find('=', 0);
	   if(stringtype::npos == n) return curSection; //不是一个正确的key=value 

	   strKey = aLineStr.substr(0,n);
	   if(stringtype::npos == n+1)
		  strVaule = "";  
	   else   
		  strVaule = aLineStr.substr(n+1);
	   AddKey(curSection.c_str(), strKey.c_str(), strVaule.c_str(), bComment ? curComment.c_str() : NULL);
	   bComment = false;
	   curComment = "";
	   return curSection;		
   }
  
   inline FINI_SECTION* FindSession(const chartype* SectionStr) const
   {
	   std::string strSession(SectionStr);
	   FINI_SECTION* pSession = nullptr;
	   auto it = _SessionArr.find(strSession);
	   if (it != _SessionArr.end())
	   {
		   pSession = it->second;
	   }
	   return pSession;
   }

   inline FINI_SECTION* AddSection(const chartype* SectionStr, const chartype* CommentStr = NULL)
   {
	   std::string strSession(SectionStr);
	   FINI_SECTION* pSession = FindSession(SectionStr);
	   if (!pSession)
	   {
		   pSession = new FINI_SECTION;
		   _SessionArr.insert(std::pair<stringtype, FINI_SECTION*>(strSession, pSession));
	   }

	   if (CommentStr) {
		   pSession->strComment = CommentStr;
		   pSession->hasComment = true;
	   }
	   else {
		   pSession->strComment = "";
		   pSession->hasComment = false;
	   }
	   pSession->strSession = strSession;

	   return pSession;
   }
   
   inline FINI_KEYVALUE* AddKey(const chartype *SectionStr, const chartype* KeyStr, const chartype* VauleStr, const chartype* strComment = NULL)
   {
	   FINI_SECTION* pSession = FindSession(SectionStr);
	   if (!pSession) return nullptr;
	   FINI_KEYVALUE* pKV = new FINI_KEYVALUE;
	   pKV->strKey = KeyStr;
	   pKV->strValue = VauleStr;
	   pKV->hasComment = !!strComment;
	   if (!!strComment) pKV->strComment = strComment;

	   pSession->values.push_back(pKV);

	   return pKV;
   }

   inline bool GetString(const chartype* SectionStr, const chartype* KeyStr, stringtype &value)  const
   {
	   FINI_SECTION* pSession = FindSession(SectionStr);
	   if(!pSession)
		   return false;

	   for (std::vector<FINI_KEYVALUE*>::iterator it=pSession->values.begin(); it != pSession->values.end(); ++it)
	   {
		   if ((*it)->strKey.compare(KeyStr) == 0)
		   {
			   value = (*it)->strValue;
			   return true;
		   }

	   }

	   return false;
   }
};
_FStdEnd
#endif//__FINI_HPP__
