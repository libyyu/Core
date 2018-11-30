
#ifndef __FDOSTRUCTFILE_HPP__
#define __FDOSTRUCTFILE_HPP__
#pragma once
#include <fstream>
#include <vector>
#include <algorithm>
#include "FDReadAWriteLock.hpp"

using namespace std;
_FStdBegin

template <class T,class U = vector<T> >
class FDoStructFile
{ 
public:
	typedef typename U::value_type value_type;
	typedef typename U::size_type size_type;
	typedef typename U  container_type;
	typedef typename U::const_iterator const_itor;
private:
	FDReadAWriteLock m_Lock; 
	U m_contian;	
	const unsigned int m_nSize;
#if PLATFORM_TARGET == PLATFORM_WINDOWS
	typedef bool (__stdcall *EnumVaulePro)(const value_type&,void*);
#else
    typedef bool (*EnumVaulePro)(const value_type&,void*);
#endif
public:
    FDoStructFile();
    ~FDoStructFile();
public:
    bool ReadFileToMem(const char* lpFileName);
	bool WriteFileFormMem(const char* lpFileName);	
	void InsertVaule(const value_type& x);
	void Clear();
	void EnumVaule(const EnumVaulePro lpEnumVaulePro,void* pParam = NULL);
};

template <class T,class U>
FDoStructFile<T,U>::FDoStructFile():m_nSize(sizeof(value_type))
{	
}
template <class T, class U>
FDoStructFile<T,U>::~FDoStructFile()
{
}
template <class T, class U>
bool FDoStructFile<T,U>::ReadFileToMem(const char* lpFileName)
{
	assert(lpFileName);
	if(!lpFileName) return false;  
	bool bVar = false;    
	
	m_Lock.EnterWrite();  
	ifstream fs(lpFileName,ios::in|ios::binary);
	if(!fs.is_open()) goto _End_Fun;
	
	{
	  bVar = true;
	  m_contian.clear();
	  value_type vule;
      while(fs.read((char*)&vule,m_nSize))
      {
    	m_contian.push_back(vule);
	  }
	  fs.close();
	  goto _End_Fun;
	}
	
_End_Fun:
    m_Lock.LeaveWrite();                	
    return bVar;	
}
template <class T,class U>
bool FDoStructFile<T,U>::WriteFileFormMem(const char* lpFileName)
{
	assert(lpFileName);
	if(!lpFileName) return false;
	bool bVar = false;
	                  
    m_Lock.EnterRead();  
	ofstream fs(lpFileName,ios::out|ios::binary);
	if(!fs.is_open()) goto _End_Fun;
	
	{
	  bVar = true;	
	  const_itor i;
	  for(i=m_contian.begin(); i!=m_contian.end(); ++i)
	  {
		value_type vale = *i;	
	    fs.write((char*)&vale,m_nSize);
	  }
	  fs.close();
	  goto _End_Fun;
	}

_End_Fun:
    m_Lock.LeaveRead();	 
	return bVar;
}
template <class T,class U>
void FDoStructFile<T,U>::EnumVaule(const EnumVaulePro lpEnumVaulePro, void *pParam = NULL)
{
	m_Lock.EnterRead();
	const_itor i;
	for (i=m_contian.begin();i!=m_contian.end();++i)
	{
		if (  lpEnumVaulePro(*i,pParam) )	break;	
	}
    m_Lock.LeaveRead();
}
template <class T,class U>
void FDoStructFile<T,U>::InsertVaule(const value_type& x)
{
	m_Lock.EnterWrite();
	m_contian.push_back(x);
	m_Lock.LeaveWrite();
}
template <class T,class U>
void FDoStructFile<T,U>::Clear()
{
	m_Lock.EnterWrite();
	m_contian.clear();
	m_Lock.LeaveWrite();
}
_FStdEnd
#endif//__FDOSTRUCTFILE_HPP__
