#include "DbOper.h"

CDbOper CDbOper::m_dbOper;

CDbOper::CDbOper(void)
{
}

CDbOper::~CDbOper(void)
{
}

CDbOper* CDbOper::Instance()
{
	return &m_dbOper;
}

int CDbOper::GetFileInfo(const char* pFileId, char* pFileNm, char* pFilePath)
{
	return 0;
}
