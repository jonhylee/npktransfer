#include "Download.h"
#include "pthread.h"

CDownload::CDownload(void)
{
}

CDownload::~CDownload(void)
{
}

int CDownload::Start()
{
	if (m_iTransStatus == TRANS_BEGIN)
	{
		return -1;
	}

	pthread_t thrd;
	pthread_create(&thrd, NULL, DwonloadProcess, this);

	return 0;
}

unsigned int CDownload::GetTotalSize()
{
	return GetRemoteFileSize();
}

void* CDownload::DwonloadProcess(void* arg)
{
	CDownload* pDownload = (CDownload*)arg;
	CURL* pCurl = curl_easy_init();

	while(pDownload->GetWorkCount() > 0)
	{
		TransAtom atom;
		if (!pDownload->GetTransferWork(atom))
		{
			continue;
		}

		char szFileNm[PATH_MAX] = {0};		//����·���ļ���
		char szUrl[URL_MAX] = {0};			//URL
		char szFullFileNm[PATH_MAX] = {0};	//��·���ļ���
		char szUrlFile[URL_MAX] = {0};		//���ļ�url

		//����������ȡ��urlFile����GetTotalFileLenth��������
		char urlFile[512] = {0};
		strcpy(szUrlFile, atom.szSvrUrl);
		strcat(szUrlFile, "?uri=file:///");
		strcat(szUrlFile, atom.szFileNm);
		strcpy(szFileNm, atom.szFileNm);//ȡ������·���ļ���
		strcpy(szFullFileNm, atom.szFilePath);
		strcat(szFullFileNm, atom.szFileNm);//��·���ļ���
		strcpy(szUrl, atom.szSvrUrl);
		strcpy(pDownload->m_szTransferFileNm, atom.szFileNm);

		unsigned int iRemoteFileSize = pDownload->GetRemoteFileSize(szUrlFile);
		unsigned int iLocalFileSize = pDownload->GetLocalFileSize(szFullFileNm);

		if (iLocalFileSize == -1)
		{
			iLocalFileSize = 0;
		}

		if(iLocalFileSize >= iRemoteFileSize)		//�����Ҫ�����ļ��Ĵ�С���ڵ��ڱ����ļ��Ĵ�С��ֱ��������һ���ļ�
		{
			pDownload->SetTransferStatus(HAD_TRANS);
			Sleep(10);
			continue;
		}
		FILE *debugFile = NULL;
		if(NULL == (debugFile = fopen("debugFile.txt", "a+")))    
			return NULL;     

		FILE* fp = fopen(szFullFileNm,"ab+");
		if(fp == NULL) //�ļ��򿪴��󣬽�����һ���ļ�������
		{
			pDownload->SetTransferStatus(OPEN_FILE_ERR);
			Sleep(10);
			continue;
		}
		curl_easy_setopt(pCurl, CURLOPT_URL, szUrlFile);
		//curl_easy_setopt(pCurl, CURLOPT_PROXY, "127.0.0.1:8888");
		curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, pDownload->GetTimeOut());
		curl_easy_setopt(pCurl, CURLOPT_HEADER, 0L);
		curl_easy_setopt(pCurl, CURLOPT_NOBODY, 0L);
		curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(pCurl, CURLOPT_COOKIE, atom.szCookie);
		curl_easy_setopt(pCurl, CURLOPT_RESUME_FROM, iLocalFileSize);
		curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, fp);
		curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, WriteFunc);
		curl_easy_setopt(pCurl, CURLOPT_NOPROGRESS, 0L);
		curl_easy_setopt(pCurl, CURLOPT_PROGRESSFUNCTION, ProgressFunc);
		curl_easy_setopt(pCurl, CURLOPT_PROGRESSDATA, pDownload);

		//��ӡdebug��Ϣ
		curl_easy_setopt(pCurl, CURLOPT_DEBUGDATA, debugFile);     
		curl_easy_setopt(pCurl, CURLOPT_DEBUGFUNCTION, DebugFun);   

		pDownload->SetTransferStatus(TRANS_BEGIN);
		if(CURLE_OK != curl_easy_perform(pCurl))
		{
			pDownload->SetTransferStatus(TRANS_FAILD);
		}
		fclose(fp);
		//Ϊ��ʹ�����ܹ�֪ͨ����ڴ�����10����
		Sleep(10);
	}
	curl_easy_cleanup(pCurl);
	Sleep(10);
	pDownload->SetTransferStatus(TRANS_END);

	return 0;
}

int CDownload::DebugFun(CURL* curl, curl_infotype type, char* str, size_t len, void* stream)     
{       
	fwrite(str, 1, len, (FILE*)stream); 
	return 0;
}


/* parse headers for Content-Length */
size_t CDownload::GetContentLengthFunc(void *ptr, size_t size, size_t nmemb, void *stream) 
{
	int r;
	long len = 0;
	r = sscanf((const char*)ptr, "Content-Length: %ld\n", &len);
	if (r) /* Microsoft: we don't read the specs */
		*((long *) stream) = len;
	return size * nmemb;
}

size_t CDownload::ProgressFunc(CDownload* pDown, double totalDown, double curDown, double totalUp, double curUp)   
{
	pDown->m_iCurTransferSize = curDown;

	return 0;
}

size_t CDownload::WriteFunc(char *str, size_t size, size_t nmemb, void *stream)
{
	return fwrite(str, size, nmemb, (FILE*)stream);
}
