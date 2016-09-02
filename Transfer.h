/********************************************************************
	文件名:     Transfer.h
    创建时间:   2014-5-14   15:06
    说明:       用作于传输基类
*********************************************************************/
#pragma once
#include <vector>
#include <pthread.h>
#include "curl.h"

using namespace std;

static const int PATH_MAX = 256;
static const int URL_MAX = 512;
static const int WORK_MAX = 10;
static const int STR_MAX = 256;

enum TRANS_STATUS
{
	NO_TRANS = 0,
	TRANS_BEGIN = 1,
	TRANS_END = 2,
	HAD_TRANS = 3,
	OPEN_FILE_ERR = 4,
	TRANS_FAILD = 5
};

typedef struct _stTransferAtom
{
	char szFileNm[PATH_MAX];
	char szFilePath[PATH_MAX];
	char szSvrUrl[URL_MAX];
	char szCookie[STR_MAX];
	bool bCompress;
} TransAtom;

class CTransfer
{
public:
	CTransfer(void);
	virtual ~CTransfer(void);

public:
	virtual int Start();
	virtual int Pasue();
	virtual int Continue();
	virtual unsigned int GetTotalSize();
	void SetTransferStatus(int iStatus);
	int GetTransferStatus();
	bool IsTransferBegin();
	bool IsTransferEnd();
	void SetTimeOut(const long lTimeOut);
	long GetTimeOut();
	void SetConnTimeOut(const long lConnTimeOut);
	long GetConnTimeOut();
	unsigned int GetLocalFileSize(const char *pFileName);
	unsigned int GetLocalFileSize();
	unsigned int GetRemoteFileSize(const char *pUrl);
	unsigned int GetRemoteFileSize();
	unsigned int GetCurTransferSize();
	const char* GetTransferFileNm();
	bool AddTransferWrok(const TransAtom& atom);
	bool GetTransferWork(TransAtom& atom);
	int GetWorkCount();
	bool CreateMultiDir(const char* pPathNm);

protected:
	vector<TransAtom> m_vecTransAtom;
	int m_iTransStatus;
	long m_lTimeOut;
	long m_lConnTimeOut;
	unsigned int m_iLocalFileSize;
	unsigned int m_iRemoteFileSize;
	unsigned int m_iCurTransferSize;
	char m_szTransferFileNm[PATH_MAX];
	pthread_mutex_t m_mutex;
};

