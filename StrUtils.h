#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include "pluginbase.h"
#include <wincrypt.h>
#include <algorithm>

class CStrUtils
{
public:
	CStrUtils(void);
	~CStrUtils(void);
public:
	//UTF-8��GB2312��ת��(��ҳ����תΪ�������)
	void U2G(const char* utf8, char* gb2312);
	//GB2312��UTF-8��ת��(�������תΪ��ҳ����)
	void G2U(const char* gb2312, char* utf8);
	//��NPVariantת��Ϊgb2312������ִ�
	void GetString(const NPVariant var, char *outGb2312, int outSize);
	//��NPStringת��Ϊgb2312������ִ�
	void GetString(const NPString npStr, char *outGb2312, int outSize);

	bool ReadFileStreem(const char* filename, BYTE *buff, int buff_size, int *real_size);
	std::string getFileExt(const char* filename);
	std::string getFileHash(const char* filename);

	//�õ����������ݵ�hashֵ
	DWORD GetHash(BYTE *pbData, DWORD dwDataLen, ALG_ID algId, LPTSTR pszHash);

	void ToUpperString(std::string &str);
	void ToLowerString(std::string &str);

	std::string GetJsFunction(const std::string &js);

	//�и��ַ���
	int SplitString(const std::string &srcStr,const std::string &splitStr, std::vector<std::string> &destVec);
};
