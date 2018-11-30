#ifndef  __FMEMPOOL_HPP__
#define  __FMEMPOOL_HPP__
#pragma once
#include <assert.h>
#include "FDReadAWriteLock.hpp"
#include <iostream>
_FUsing(std)
/*****************************************************************************************************
  ±¾ÀàÎªÄÚ´æ³ØÀà£¬ÒÔÁ´±íÐÎÊ½¹ÜÀí£¬ÊµÔòÕ»Ê½£¬´ÓÁ´±íÍ·´¦²åÈëÓëÒÆ³ý£¬
  Í¬Ê±Àà±¾ÉíÒ²ÎªÒ»¸öÁ´±íµÄ½Úµã;m_pNext¼ÇÂ¼ÏÂÒ»¸öÁ´±í
  ÀàÖÐÈÎºÎ´óÐ¡ÄÚ´æ¶¼¿ÉÒÔ³É¹¦ÉêÇë£¬µ«Ö»´æ·ÅÐ¡ÓÚMaxSaveSizeµÄÄÚ´æ¿é£¬ÆäËüÖ±½ÓÉ¾³ý

******************************************************************************************************/
_FDeclsBegin
_FStdBegin
typedef struct _F_Node_
{
	unsigned int nLen;
	struct _F_Node_* next;
}F_Node;
const unsigned int NodeSize = sizeof(F_Node);
const unsigned int MaxSaveBlock = 1024*1024*1;  //¶¨Òå´óÓÚ1MÄÚ´æÊ±ÄÚ´æ³Ø²»¹ÜÀí

class FMemPool
{
public:
	FMemPool(const unsigned int& nDataSize,const unsigned int& nMaxSize=0):m_nBlockSize(nDataSize/*+NodeSize*/),
		m_nMaxSize(nMaxSize),m_pFirst(NULL),m_pNext(NULL),m_nInCount(0),m_nOutConut(0){};  
	~FMemPool()
	{
		m_Lock.EnterWrite();   
		{
			if (m_pFirst) DeleteNext(m_pFirst);
			m_pFirst = NULL;
			if (m_pNext) delete m_pNext;//×¢Òâ Êµ¼ÊÊÇµÝ¹éµ÷ÓÃ
			m_pNext = NULL;
		}
		m_Lock.LeaveWrite();
		//cout<<"ÄÚ´æ³Ø£¬¿é´óÐ¡£º"<<m_nBlockSize<<"´æ¸öÊý£º"<<m_nInCount<<"Å×³ö¸öÊý£º"<<m_nOutConut<<endl;
	};
public:
	void* GetBuf(const unsigned int &nSize)                    //°´Ö¸¶¨´óÐ¡´ÓÄÚ´æ³ØÖÐ»ñÈ¡ÄÚ´æ£¨nSize£©
	{
		void* pResutl = NULL;
		F_Node* pNewNode = NULL;

		if ( (nSize+NodeSize) <= m_nBlockSize)  //ÄÚ´æ¿é´óÐ¡ > £¨ÓÃ»§ÇëÇóÄÚ´æ´óÐ¡ + Á´±íÖ¸Õë´óÐ¡)
		{		                               //±íÊ¾±¾¶ÔÏóµÄ´óÐ¡¿ÉÒÔÂú×ãÉêÇëÐèÇó£¬ÉêÇë´Ó±¾Á´±íÖÐ»ñÈ¡
			m_Lock.EnterWrite();
			{
				if (m_pFirst)                  //Á´±í·Ç¿Õ£¬´ÓÁ´±í»ñÈ¡½Úµã
				{                             //×¢Òâ²»¿¼ÂÇ¿Õ½ÚµãÇé¿ö£¬ÒòÎª¿Õ±íÊ±£¬ËµÃ÷±íÖÐÃ»ÓÐÄÚ´æ£¬ÔòÓ¦¸Ã¼ì²éÊÇ·ñ¿ÉÒÔÂú×ãÉêÇëÌõ¼þ£¬Èç²»Âú×ãÖ±½Ó·´»ØNULL					  				
					pNewNode = m_pFirst;
					m_pFirst = m_pFirst->next;
					pNewNode->next = NULL;				
					--m_nInCount;
					++m_nOutConut;
					pResutl = (char*)pNewNode + NodeSize;  //°ÑÓÃ»§Ê¹ÓÃÄÚ´æºóÒÆ£¬ÒòÎªÄÚ´æÖÐÇ°ÃæNodeSize´óÐ¡µÄÄÚ´æ´æ·ÅÁËNodeÖÐµÄ¼ÇÂ¼
				}
				else if (0 == m_nMaxSize || m_nOutConut < m_nMaxSize)
				{
					pNewNode = (F_Node*)malloc(m_nBlockSize);
					if (pNewNode)
					{					
						pNewNode->nLen = m_nBlockSize;       //½áµãÖÐ¼ÇÂ¼±¾ÄÚ´æ¿é´óÐ¡£¬·½±ãÊÍ·ÅÊ±»Ø¹éµ½±¾Á´±íÖÐ
						pNewNode->next = NULL;				 
						pResutl = (char*)pNewNode + NodeSize;//°ÑÓÃ»§Ê¹ÓÃÄÚ´æºóÒÆ£¬ÒòÎªÄÚ´æÖÐÇ°ÃæNodeSize´óÐ¡µÄÄÚ´æ´æ·ÅÁËNodeÖÐµÄ¼ÇÂ¼
						++m_nOutConut;	
					}
				}
			}
			m_Lock.LeaveWrite();
		}
		else   //·Ç±¾Á´±í¹ÜÀí£¬ÇëÇóÏÂ¸öÁ¬±í´¦Àí
		{
			m_Lock.EnterWrite();
			if (!m_pNext)
				m_pNext = new FMemPool(2*(m_nBlockSize/*-NodeSize*/),m_nMaxSize); //×¢ÒâÎª±¶ÊýÉêÇë
			if(m_pNext)
				pResutl = m_pNext->GetBuf(nSize);
			m_Lock.LeaveWrite();
		}	
		return pResutl;
	};
	bool FreeBuf(void* pBuf,bool bDelete)                      //ÊÍ·ÅÄÚ´æ£¬bDelete Ö¸Ê¾ÊÇ·ñÕæÕýÉ¾³ýÄÚ´æ	
	{
		if (!pBuf) return false;
		bool bResult = false;
		F_Node* pNode = GetNode(pBuf);
		if(!pNode) return false;

		if (m_nBlockSize == pNode->nLen)   //±¾Á´±í¹ÜÀíÄÚ´æ
		{
			if (m_nBlockSize >= MaxSaveBlock || bDelete) //Âú×ãÖ±½ÓÉ¾³ýÌõ¼þ£¬ÔòÖ±½ÓÉ¾³ý
			{
				free(pNode);
				--m_nOutConut;
			}
			else                                        //´æÈëÁ´±í    
			{
				m_Lock.EnterWrite();//¼ÓËø£¬Ð´·ÃÎÊ             
				{
					pNode->next =  m_pFirst;
					m_pFirst = pNode;
				}
				m_Lock.LeaveWrite();
				--m_nOutConut;
				++m_nInCount;
			}
			bResult = true;   //×¢ÒâÕâÀïÖ±½Ó·µ»ØTrue ,ÒòÎª²Ù×÷¿Ï¶¨³É¹¦
		}
		else //·Ç±¾Á´±í¹ÜÀí
		{
			m_Lock.EnterRead();   //¼Ó¶ÁËø£¬¶Ôm_pNext¶Á·ÃÎÊ
			{
				if (m_pNext)			
					bResult = m_pNext->FreeBuf(pBuf,bDelete);			
			}
			m_Lock.LeaveRead();
		}

		return bResult;
	}
	bool IsIn(const void* pBuf,const unsigned int &nNewSize)
	{
		assert(pBuf);
		if (0 >= nNewSize) return false;
		if ( (FMemPool::GetNode(pBuf)->nLen - NodeSize) >= nNewSize )
			return true;
		else 
			return false;
	}
	static unsigned int GetBufLen(const void* pBuf)
	{
		return GetNode(pBuf)->nLen;
	};
	void PrintInfo()
	{
		//ÔÝ²»ÊµÏÖ
	};
private:
	void DeleteNext(F_Node* pNext)                      //µÝ¹éÉ¾³ýÁ´±íÖÐµÄ½Úµã
	{
		//±¾º¯ÊýÀàË½ÓÐ£¬ÀàÖÐÆäËüº¯Êýµ÷ÓÃ£¨¼ÓËø£©£¬ËùÒÔ´Ë´¦²»ÓÃ¼ÓËø
		if( !pNext) return;
		F_Node* pNode = pNext;
		F_Node* pTmpNode = NULL;
		while(1)
		{
			pTmpNode = pNode->next;
			free(pNode);
			--m_nInCount;
			if (!pTmpNode) break;
			pNode = pTmpNode;
		}
	}
	static F_Node* GetNode(const void* pBuf)
	{                               //¾²Ì¬º¯Êý²»ÓÃ¼ÓËø
		assert(pBuf);
		F_Node* pTmpNode = (F_Node*)((char*)pBuf-NodeSize);
		return pTmpNode;
	};
private:
	const unsigned int  m_nBlockSize;                  //ÄÚ´æ¿é´óÐ¡  ÒÔ16KÎª³õÊ¹´óÐ¡£¬ÒÔ±¶ÊýµÝÔö(³£Á¿·ÀÖ¹ÐÞ¸Ä)
	const unsigned int  m_nMaxSize;                    //Á´±í¹ÜÀíµÄ×îÄÚ´æ¿é×î´ó¸öÊý
	unsigned long       m_nOutConut;                   //Å×³öµÄÄÚ´æ¿é¸öÊý
	unsigned long       m_nInCount;                    //ÄÚ´æ³ØÖÐ´æ´¢µÄÄÚ´æ¿é¸öÊý
	F_Node*         m_pFirst;                            //´ËÁ¬±íµÄÍ·½Úµã
	FDReadAWriteLock m_Lock;                         //¶à¶Áµ¥¶¨Ëø±£»¤±äÁ¿
	FMemPool*   m_pNext;                             //±¾Á´±íµÄÏÂÒ»¸öÁ´±íÖ¸Õë
};

class FMemPoolMgr
{
public:
	FMemPoolMgr(unsigned int nMinSize = 8,const unsigned int nMaxSize = 0)
	{
		m_pHead = new FMemPool((nMinSize<16 ? nMinSize =16:nMinSize),nMaxSize) ; //ÒÔ×îÐ¡ÄÚ´æ¿é4bytes ³õÊ¹¹ÜÀíÁ¬±íÍ·£»
	};
	~FMemPoolMgr()
	{
		if (m_pHead)
		{
			delete m_pHead;
			m_pHead = NULL;
		}
	};
public:
	void* GetBuf(unsigned int &nSize)
	{
		if (0 >= nSize) return NULL;

		void* pResult = NULL;
		if (m_pHead)
		{
			pResult = m_pHead->GetBuf(nSize);
		}
		return pResult;
	};                                             
	void* ReGetBuf(void* pOldBuf,unsigned int& nNewSize,bool bCopyOldData = true)
	{
		assert(pOldBuf);

		void* pResutl = NULL;
		if (0 >= nNewSize)
		{
			return pResutl;
		}	
		if (m_pHead->IsIn(pOldBuf,nNewSize))
		{
			pResutl = pOldBuf;
		}
		else
		{
			pResutl = m_pHead->GetBuf(nNewSize);
			if (pResutl && pOldBuf)
			{
				if(bCopyOldData)
					memcpy(pResutl,pOldBuf,FMemPool::GetBufLen(pOldBuf));
			}
		}
		return pResutl;
	}; 
	bool  Free(void* pBuf,bool bDelete = false)
	{
		if (!pBuf) return false;
		if(m_pHead)
			m_pHead->FreeBuf(pBuf,bDelete);
		return true;
	};
private:
	FMemPool* m_pHead;      //¹ÜÀíÁ´±íÍ·½áµã
};
_FStdEnd
_FDeclsEnd
#endif//__FMEMPOOL_HPP__
