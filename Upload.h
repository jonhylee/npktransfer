/********************************************************************
	文件名:     Upload.h
    创建时间:   2014-5-14   15:08
    说明:       上传工作类
*********************************************************************/
#pragma once
#include "transfer.h"

class CUpload :
	public CTransfer
{
public:
	CUpload(void);
	~CUpload(void);

public:
	virtual int Start();
	unsigned int GetTotalSize();

public:
	static void* UploadProcess(void* arg);
	static int DebugFun(CURL* curl, curl_infotype type, char* str, size_t len, void* stream);
	static size_t GetContentLengthFunc(void *ptr, size_t size, size_t nmemb, void *stream);
	static size_t ProgressFunc(CUpload* pUp, double totalDown, double curDown, double totalUp, double curUp);
	static size_t ReadFunc(void *ptr, size_t size, size_t nmemb, void *stream);
};
