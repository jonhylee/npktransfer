#include "Upload.h"
#include "pthread.h"

CUpload::CUpload(void)
{
}

CUpload::~CUpload(void)
{
}

int CUpload::Start()
{
	if (m_iTransStatus == TRANS_BEGIN)
	{
		return -1;
	}

	pthread_t thrd;
	pthread_create(&thrd, NULL, UploadProcess, this);

	return 0;
}

unsigned int CUpload::GetTotalSize()
{
	return GetLocalFileSize();
}


void* CUpload::UploadProcess(void* arg)
{
	CUpload* pUp= (CUpload*)arg;
	CURL* pCurl = curl_easy_init();

	struct curl_httppost *formpost=NULL;
	struct curl_httppost *lastptr=NULL;
	struct curl_slist *headerlist=NULL;

	while(pUp->GetWorkCount() > 0)
	{
		TransAtom atom;
		if (!pUp->GetTransferWork(atom))
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
		strcpy(pUp->m_szTransferFileNm, atom.szFileNm);

		unsigned int iRemoteFileSize = pUp->GetRemoteFileSize(szUrlFile);
		unsigned int iLocalFileSize = pUp->GetLocalFileSize(szFullFileNm);

		//����ϴ����ļ���С���ڵ��ڱ����ļ��Ĵ�С��˵��������Ѿ���������ļ��ˣ����ϴ���һ���ļ�
 		if(iRemoteFileSize >= iLocalFileSize)		
		{
			pUp->SetTransferStatus(HAD_TRANS);
			Sleep(10);
 			continue;
 		}

		FILE* fp = fopen(szFullFileNm,"rb");
		if(fp == NULL) //�ļ��򿪴��󣬽�����һ���ļ����ϴ�
		{
			pUp->SetTransferStatus(OPEN_FILE_ERR);
			Sleep(10);
			continue;
		}

		//�ϴ�ʱ��Զ���ļ���С��Ϊ��ǰ�Դ����ļ���С���Ա�����ʱ��������ȷ��ʾ
		pUp->m_iCurTransferSize = iRemoteFileSize;

		fseek( fp, iRemoteFileSize, SEEK_SET );

		//����صĵ�����Ϣ��ӡ��dubugFile.txt��     
		/*FILE *debugFile = NULL;
		if(NULL == (debugFile = fopen("debugFile.txt", "a+")))    
			return NULL;    */ 

       //���� CURLFORM_STREAM
		formpost = NULL;
		lastptr = NULL;
		curl_formadd(&formpost,
		   &lastptr,
		   CURLFORM_COPYNAME, "upload",
		   CURLFORM_FILENAME, szFileNm,
		   CURLFORM_STREAM, fp,
		   CURLFORM_CONTENTSLENGTH, iLocalFileSize,
		   CURLFORM_END);

		curl_easy_setopt(pCurl, CURLOPT_URL, szUrl);
		/* Now specify we want to POST data */ 
		curl_easy_setopt(pCurl, CURLOPT_POST, 1L);
		curl_easy_setopt(pCurl, CURLOPT_NOBODY, 0L);
		curl_easy_setopt(pCurl, CURLOPT_HEADER, 0L);
		//curl_easy_setopt(pCurl, CURLOPT_PROXY, "127.0.0.1:8888");//fiddler����������
		curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, pUp->GetTimeOut());
		curl_easy_setopt(pCurl, CURLOPT_CONNECTTIMEOUT, pUp->GetConnTimeOut());
		curl_easy_setopt(pCurl, CURLOPT_COOKIE, atom.szCookie);
		curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(pCurl, CURLOPT_READDATA, fp);
		curl_easy_setopt(pCurl, CURLOPT_READFUNCTION, ReadFunc);
		curl_easy_setopt(pCurl, CURLOPT_HTTPPOST, formpost);
		curl_easy_setopt(pCurl, CURLOPT_NOPROGRESS, 0L);
		curl_easy_setopt(pCurl, CURLOPT_PROGRESSFUNCTION, ProgressFunc);
		curl_easy_setopt(pCurl, CURLOPT_PROGRESSDATA, pUp);
		curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1L);// �������CURL����ÿһ����������飬�������ѡ��Ϊһ������ֵ��
		//��ӡdebug��Ϣ
		/*curl_easy_setopt(pCurl, CURLOPT_DEBUGDATA, debugFile);     
	    curl_easy_setopt(pCurl, CURLOPT_DEBUGFUNCTION, DebugFun);   */

		/*
		  Using POST with HTTP 1.1 implies the use of a "Expect: 100-continue"
		  header.  You can disable this header with CURLOPT_HTTPHEADER as usual.
		  NOTE: if you want chunked transfer too, you need to combine these two
		  since you can only set one list of headers with CURLOPT_HTTPHEADER. */ 

		/* A less good option would be to enforce HTTP 1.0, but that might also
		   have other implications. */ 
		headerlist = curl_slist_append(headerlist, "Expect:");
		curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, headerlist);
		/* use curl_slist_free_all() after the *perform() call to free this
		list again */ 
		pUp->SetTransferStatus(TRANS_BEGIN);
	    int iRet = curl_easy_perform(pCurl);
		if( iRet != CURLE_OK)
		{
			pUp->SetTransferStatus(TRANS_FAILD);
		}

		fclose(fp);
		//fclose(debugFile);

		Sleep(10);
	}
	curl_easy_cleanup(pCurl);
	/* then cleanup the formpost chain */ 
	curl_formfree(formpost);
	/* free slist */ 
	curl_slist_free_all (headerlist);

	//Ϊ��ʹ�����ܹ�֪ͨ����ڴ�����10����
	Sleep(10);

	pUp->SetTransferStatus(TRANS_END);

	return 0;
}

int CUpload::DebugFun(CURL* curl, curl_infotype type, char* str, size_t len, void* stream)     
{       
	fwrite(str, 1, len, (FILE*)stream); 
	return 0;
}


/* parse headers for Content-Length */
size_t CUpload::GetContentLengthFunc(void *ptr, size_t size, size_t nmemb, void *stream) 
{
	int r;
	long len = 0;
	r = sscanf((const char*)ptr, "Content-Length: %ld\n", &len);
	if (r) /* Microsoft: we don't read the specs */
		*((long *) stream) = len;
	return size * nmemb;
}

size_t CUpload::ProgressFunc(CUpload* pUp, double totalDown, double curDown, double totalUp, double curUp)   
{
	pUp->m_iCurTransferSize = curUp;
	
	return 0;
}

/* read data to upload */
size_t CUpload::ReadFunc(void *ptr, size_t size, size_t nmemb, void *stream)
{
	FILE *f = (FILE*)stream;
	size_t n;
	if (ferror(f))
		return CURL_READFUNC_ABORT;

	n = fread(ptr, size, nmemb, f) * size;
	return n;
}