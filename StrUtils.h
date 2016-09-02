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
	//UTF-8到GB2312的转换(网页数据转为插件数据)
	void U2G(const char* utf8, char* gb2312);
	//GB2312到UTF-8的转换(插件数据转为网页数据)
	void G2U(const char* gb2312, char* utf8);
	//从NPVariant转化为gb2312编码的字串
	void GetString(const NPVariant var, char *outGb2312, int outSize);
	//从NPString转化为gb2312编码的字串
	void GetString(const NPString npStr, char *outGb2312, int outSize);

	bool ReadFileStreem(const char* filename, BYTE *buff, int buff_size, int *real_size);
	std::string getFileExt(const char* filename);
	std::string getFileHash(const char* filename);

	//得到二进制数据的hash值
	DWORD GetHash(BYTE *pbData, DWORD dwDataLen, ALG_ID algId, LPTSTR pszHash);

	void ToUpperString(std::string &str);
	void ToLowerString(std::string &str);

	std::string GetJsFunction(const std::string &js);

	//切割字符串
	int SplitString(const std::string &srcStr,const std::string &splitStr, std::vector<std::string> &destVec);
};
