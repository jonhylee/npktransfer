#include <io.h>
#include "Transfer.h"

CTransfer::CTransfer(void)
:m_vecTransAtom()
,m_iTransStatus(NO_TRANS)
,m_lTimeOut(0)
,m_lConnTimeOut(0)
,m_iLocalFileSize(0)
,m_iRemoteFileSize(0)
,m_iCurTransferSize(0)
{
	memset(m_szTransferFileNm, 0, PATH_MAX);
	curl_global_init (CURL_GLOBAL_ALL);
	pthread_mutex_init(&m_mutex, NULL);
}

CTransfer::~CTransfer(void)
{
	curl_global_cleanup();
	pthread_mutex_destroy(&m_mutex);
}

int CTransfer::Start()
{
	return 0;
}

int CTransfer::Pasue()
{
	return 0;
}

int CTransfer::Continue()
{
	return 0;
}

unsigned int CTransfer::GetTotalSize()
{
	return 0;
}

void CTransfer::SetTransferStatus(int iStatus)
{
	pthread_mutex_lock(&m_mutex);
	m_iTransStatus = iStatus;
	pthread_mutex_unlock(&m_mutex);
}

int CTransfer::GetTransferStatus()
{
	pthread_mutex_lock(&m_mutex);
	int iStatus = m_iTransStatus;
	pthread_mutex_unlock(&m_mutex);
	return iStatus;
};

bool CTransfer::IsTransferBegin()
{
	pthread_mutex_lock(&m_mutex);
	bool ret = m_iTransStatus == TRANS_BEGIN ? true : false;
	pthread_mutex_unlock(&m_mutex);

	return ret;
}

bool CTransfer::IsTransferEnd()
{
	pthread_mutex_lock(&m_mutex);
	bool ret = m_iTransStatus == TRANS_END ? true : false;
	pthread_mutex_unlock(&m_mutex);

	return ret;
}

void CTransfer::SetTimeOut(const long lTimeOut)
{
	m_lTimeOut = lTimeOut;
}

long CTransfer::GetTimeOut()
{
	return m_lTimeOut;
}

void CTransfer::SetConnTimeOut(const long lConnTimeOut)
{
	m_lConnTimeOut = lConnTimeOut;
}

long CTransfer::GetConnTimeOut()
{
	return m_lConnTimeOut;
}

unsigned int CTransfer::GetLocalFileSize(const char *pFileName)
{
	FILE* fp = fopen(pFileName, "rb");

	if(fp != NULL)
	{
		fseek(fp, 0, SEEK_END); 
		m_iLocalFileSize = filelength(fileno(fp));
		rewind(fp);
		return m_iLocalFileSize;
	}

	return -1;
}

unsigned int CTransfer::GetLocalFileSize()
{
	return m_iLocalFileSize;
}

unsigned int CTransfer::GetRemoteFileSize(const char *pUrl)
{
	double dRemoteFileSize = 0;
	CURL* pCurl = curl_easy_init();

	if (NULL == pCurl)
	{
		return -1;
	}
	curl_easy_setopt(pCurl, CURLOPT_URL, pUrl);
	curl_easy_setopt(pCurl, CURLOPT_HEADER, 1L);
	curl_easy_setopt(pCurl, CURLOPT_NOBODY, 1L);

	int iRet = curl_easy_perform(pCurl);
	if( iRet == CURLE_OK)
	{
		curl_easy_getinfo(pCurl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &dRemoteFileSize);
	}
	else
	{
		dRemoteFileSize = -1;
	}
	curl_easy_cleanup(pCurl);

	m_iRemoteFileSize = dRemoteFileSize;

	return dRemoteFileSize;
}

unsigned int CTransfer::GetRemoteFileSize()
{
	return m_iRemoteFileSize;
}

unsigned int CTransfer::GetCurTransferSize()
{
	return m_iCurTransferSize;
}

const char* CTransfer::GetTransferFileNm()
{
	return m_szTransferFileNm;
}

bool CTransfer::AddTransferWrok(const TransAtom& atom)
{
	if (m_vecTransAtom.size() >= WORK_MAX)
	{
		return false;
	}

	m_vecTransAtom.push_back(atom);

	return true;
}

bool CTransfer::GetTransferWork(TransAtom& atom)
{
	if (m_vecTransAtom.empty())
	{
		return false;
	}

	atom = m_vecTransAtom.front();
	vector<TransAtom>::iterator it = m_vecTransAtom.begin();
	m_vecTransAtom.erase(it);

	return true;
}

int CTransfer::GetWorkCount()
{
	return m_vecTransAtom.size();
}

bool CTransfer::CreateMultiDir(const char* pPathNm)
{
	if(pPathNm == NULL) 
		return false;

	char szPath[PATH_MAX] = {0};
	strcpy(szPath, pPathNm);
	int iIndex = 0;
	int iLen = strlen(pPathNm);
	char szCurPath[PATH_MAX] = {0};
	WIN32_FIND_DATA swf;

	if(szPath[iLen - 1] != '\\')	//最后一个非0字符不是‘\\’则加上
	{
		szPath[iLen] = '\\';
	}

	while(szPath[iIndex] != '\0')
	{
		if(szPath[iIndex] == ':')
		{
			iIndex+=2;
			continue;
		}
		if(szPath[iIndex] == '\\')
		{
			memcpy(szCurPath, szPath, iIndex);
			szCurPath[iIndex] = '\0';

			if(FindFirstFile(szCurPath, &swf) == INVALID_HANDLE_VALUE) //目录不存在就创建
			{
				if(!CreateDirectory(szCurPath, NULL))
				{
					return FALSE;
				}
			}
		}
		iIndex++;
	}
	return TRUE;
}