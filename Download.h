/********************************************************************
	文件名:     Download.h
    创建时间:   2014-5-14   15:33
    说明:       下载工作类
*********************************************************************/
#pragma once
#include "transfer.h"

class CDownload :
	public CTransfer
{
public:
	CDownload(void);
	~CDownload(void);

public:
	int Start();
	unsigned int GetTotalSize();

public:
	static void* DwonloadProcess(void* arg);
	static int DebugFun(CURL* curl, curl_infotype type, char* str, size_t len, void* stream);
	static size_t GetContentLengthFunc(void *ptr, size_t size, size_t nmemb, void *stream);
	static size_t ProgressFunc(CDownload* pDown, double totalDown, double curDown, double totalUp, double curUp);
	static size_t WriteFunc(char *str, size_t size, size_t nmemb, void *stream);
};
