/********************************************************************
	�ļ���:     DbOper.h
    ����ʱ��:   2014-5-15   16:21
    ˵��:       ���ݿ������
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
