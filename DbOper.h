/********************************************************************
	文件名:     DbOper.h
    创建时间:   2014-5-15   16:21
    说明:       数据库操作类
*********************************************************************/
#pragma once

class CDbOper
{
public:
	~CDbOper(void);

private:
	CDbOper(void);
	CDbOper(CDbOper& ohs);

public:
	static CDbOper* Instance();
	int GetFileInfo(const char* pFileId, char* pFileNm, char* pFilePath);
	
private:
	static CDbOper m_dbOper;
};
