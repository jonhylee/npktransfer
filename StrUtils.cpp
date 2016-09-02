#include "StrUtils.h"

CStrUtils::CStrUtils(void)
{
}

CStrUtils::~CStrUtils(void)
{
}

void CStrUtils::U2G(const char* utf8, char* gb2312)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len+1];
	memset(wstr, 0, len+1);
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);
	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	memset(gb2312, 0, sizeof(gb2312));
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, gb2312, len, NULL, NULL);
	if(wstr) delete[] wstr;
}

void CStrUtils::G2U(const char* gb2312, char* utf8)
{
	int len = MultiByteToWideChar(CP_ACP, 0, gb2312, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len+1];
	memset(wstr, 0, len+1);
	MultiByteToWideChar(CP_ACP, 0, gb2312, -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	memset(utf8, 0, sizeof(utf8));
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, utf8, len, NULL, NULL);
	if(wstr) delete[] wstr;
}

void CStrUtils::GetString(const NPVariant var, char *outGb2312, int outSize)
{
	NPString npStr;
	npStr = NPVARIANT_TO_STRING(var);
	this->GetString(npStr, outGb2312, outSize);
}

void CStrUtils::GetString(const NPString npStr, char *outGb2312, int outSize)
{
	char *strBuf = new char[outSize];
	memset(strBuf, 0, outSize);
	int utf8Len = sizeof(NPUTF8) * npStr.UTF8Length;
	utf8Len = (utf8Len<outSize)?utf8Len:outSize;
	memcpy(strBuf, npStr.UTF8Characters, sizeof(NPUTF8) * npStr.UTF8Length);
	CStrUtils su;
	su.U2G(strBuf, outGb2312);
	if(strBuf) delete[] strBuf;
}

std::string CStrUtils::getFileExt(const char* filename)
{
	std::string strfile(filename);
	std::string fgf(".");
	int pos = strfile.find_last_of(fgf);
	strfile.replace(0, pos+1, std::string(""));
	return strfile;
}

bool CStrUtils::ReadFileStreem(const char* filename, BYTE *buff, int buff_size, int *real_size)
{
	FILE *fp;
	if((fp=fopen(filename,"r+b"))==NULL) return false;
	fseek(fp,0,SEEK_END);
	int fileSize=ftell(fp);
	*real_size = fileSize;
	if((buff_size-1)<fileSize)
	{
		fclose(fp);
		return false;
	}
	fseek(fp,0,SEEK_SET);
	fread(buff, sizeof(BYTE), fileSize, fp);
	fclose(fp);
	return true;
}

std::string CStrUtils::getFileHash(const char* filename)
{
#define FILE_BUFF_SIZE (1024 * 1024 * 1) //1M
	BYTE *buff = new BYTE[FILE_BUFF_SIZE];
	int fileSize = 0;
	ReadFileStreem(filename, buff, FILE_BUFF_SIZE, &fileSize);
	buff[fileSize] = 0;
	char hash[128] = {0};
	GetHash(buff, fileSize, CALG_MD5, hash);
	delete []buff;
	return std::string(hash);
}

DWORD CStrUtils::GetHash(BYTE *pbData, DWORD dwDataLen, ALG_ID algId, LPTSTR pszHash)
{
	DWORD dwReturn = 0;
	HCRYPTPROV hProv;
	if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
		return (dwReturn = GetLastError());

	HCRYPTHASH hHash;
	if(!CryptCreateHash(hProv, algId, 0, 0, &hHash))
	{
		dwReturn = GetLastError();
		CryptReleaseContext(hProv, 0);
		return dwReturn;
	}

	if(!CryptHashData(hHash, pbData, dwDataLen, 0))
	{
		dwReturn = GetLastError();
		CryptDestroyHash(hHash);
		CryptReleaseContext(hProv, 0);
		return dwReturn;
	}

	DWORD dwSize;
	DWORD dwLen = sizeof(dwSize);
	CryptGetHashParam(hHash, HP_HASHSIZE, (BYTE*)(&dwSize), &dwLen, 0);

	BYTE* pHash = new BYTE[dwSize];
	dwLen = dwSize;
	CryptGetHashParam(hHash, HP_HASHVAL, pHash, &dwLen, 0);

	lstrcpy(pszHash, "");
	TCHAR szTemp[3];
	for (DWORD i = 0; i < dwLen; ++i)
	{
		wsprintf(szTemp, "%02X", pHash[i]);
		lstrcat(pszHash, szTemp);
	}
	delete [] pHash;

	CryptDestroyHash(hHash);
	CryptReleaseContext(hProv, 0);
	return dwReturn;
}

void CStrUtils::ToUpperString(std::string &str)
{  
	transform(str.begin(), str.end(), str.begin(), (int (*)(int))toupper);  
}

void CStrUtils::ToLowerString(std::string &str)
{  
	transform(str.begin(), str.end(), str.begin(), (int (*)(int))tolower);  
}


//此函数需要修改，没包括完所有输入情况
std::string CStrUtils::GetJsFunction(const std::string &js)
{
	std::string ret = js;
	int x = ret.find("javascript:");
	if(x==0)
		ret = ret.substr(11, ret.length() - 11);
	x = ret.find("(");
	if(x>0)
		ret = ret.substr(0, x);
	return ret;
}

int CStrUtils::SplitString(const std::string &srcStr,const std::string &splitStr, std::vector<std::string> &destVec)
{
	if(srcStr.size()==0)
	{   
		return 0;
	}   
	size_t oldPos,newPos;
	oldPos=0;
	newPos=0;
	std::string tempData;
	while(1)
	{   
		newPos=srcStr.find(splitStr,oldPos);
		if(newPos!=std::string::npos)
		{   
			tempData = srcStr.substr(oldPos,newPos-oldPos);
			destVec.push_back(tempData);
			oldPos=newPos+splitStr.size();
		}   
		else if(oldPos<=srcStr.size())
		{   
			tempData= srcStr.substr(oldPos);
			destVec.push_back(tempData);
			break;
		}   
		else
		{   
			break;
		}   
	}   
	return 0;
}