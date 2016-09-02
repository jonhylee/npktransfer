#include "Plugin.h"
#include "StrUtils.h"
#include "JsonParsor.h"
#include "Upload.h"
#include "Download.h"
#include "DbOper.h"
#include <pthread.h>

#define MAX_STRING_BUF_SIZE 256

////// functions /////////
NPError NS_PluginInitialize()
{
	return NPERR_NO_ERROR;
}

void NS_PluginShutdown()
{
}

nsPluginInstanceBase * NS_NewPluginInstance(nsPluginCreateData * aCreateDataStruct)
{
	if(!aCreateDataStruct)
		return NULL;

	CPlugin * plugin = new CPlugin(aCreateDataStruct);
	return plugin;
}

void NS_DestroyPluginInstance(nsPluginInstanceBase * aPlugin)
{
	if(aPlugin)
		delete (CPlugin *)aPlugin;
}
////// CPlugin /////////
CPlugin::CPlugin(nsPluginCreateData* data) : nsPluginInstanceBase(),
m_bInitialized(FALSE),
m_pScriptableObject(NULL),
m_pUpload(NULL),
m_pDownload(NULL)
{
	m_pNPInstance = data->instance;
	m_pNPInstance->pdata = this;

	//解析初始化参数到m_initedPramaMap
	CStrUtils su;
	for(int i=0; i < data->argc; i++)
	{
		char buf[MAX_STRING_BUF_SIZE];
		NPString npStr;
		npStr.UTF8Characters = data->argv[i];
		npStr.UTF8Length = strlen(data->argv[i]);
		su.GetString(npStr, buf, MAX_STRING_BUF_SIZE);
		m_initedPramaMap.insert(std::make_pair(std::string(data->argn[i]),std::string(buf)));
	}

	//得到程序路径
	TCHAR app_path[512+1] = {0};
	GetModuleFileName(NULL, app_path, sizeof(app_path)-1);
	std::string apppathKey("app_path");
	std::string apppath(app_path);
	std::string fgf("\\");
	int pos = apppath.find_last_of(fgf);
	apppath.replace(pos, apppath.length() - pos, std::string("\\"));
	m_app_folder = apppath;
}

CPlugin::~CPlugin()
{
	if (NULL != m_pUpload)
	{
		delete m_pUpload;
		m_pUpload = NULL;
	}

	if (NULL != m_pDownload)
	{
		delete m_pDownload;
		m_pDownload = NULL;
	}

	if(m_pScriptableObject != NULL)
		NPN_ReleaseObject(m_pScriptableObject);
}

HWND CPlugin::GetHWND()
{
	return m_hWnd;
}
NPError CPlugin::GetCookie(const char *pUrl, char **pValue, uint32_t *uiLen)
{
	return NPN_GetValueForURL(this->m_pNPInstance, NPNURLVCookie, pUrl, pValue, uiLen);
}

BOOL CPlugin::GetCookie(std::string &cookies)
{
	NPObject *npwindow;
	NPVariant docObj;

	NPN_GetValue(m_pNPInstance, NPNVWindowNPObject, &npwindow);

	NPIdentifier location = NPN_GetStringIdentifier("document");
	bool bRet = NPN_GetProperty(m_pNPInstance, npwindow, location, &docObj);
	if(!bRet)
	{
		NPN_ReleaseObject(npwindow);
		return FALSE;
	}
	NPObject* documentOBJ = NPVARIANT_TO_OBJECT(docObj);

	NPIdentifier cookieId = NPN_GetStringIdentifier("cookie");
	NPVariant cookieVar;
	bRet = NPN_GetProperty(m_pNPInstance, documentOBJ, cookieId,
		&cookieVar);
	if(!bRet)
	{
		NPN_ReleaseObject(documentOBJ);
		NPN_ReleaseObject(npwindow);
		return FALSE;
	}
	CStrUtils su;
	char buf[MAX_STRING_BUF_SIZE];
	su.GetString(cookieVar, buf, MAX_STRING_BUF_SIZE);
	cookies.append(buf);
	NPN_ReleaseObject(documentOBJ);
	NPN_ReleaseObject(npwindow);
	return TRUE;

}

BOOL CPlugin::Upload(const char* pParam)
{
	CStrUtils su;
	JsonParsor parsor;
	std::map<std::string, std::string> mapParam;
	if (!parsor.Parse(pParam, mapParam))
	{
		return FALSE;
	}

	std::string strFileList = mapParam["fileList"];
	std::string strSvrUrl = mapParam["svrHost"];
	std::string strCompress = mapParam["isCompress"];

	std::vector<std::string> vecFile;
	if (0 != su.SplitString(strFileList, ",", vecFile))
	{
		return -1;
	}

	m_pUpload = new CUpload();

	std::vector<std::string>::iterator it = vecFile.begin();
	std::vector<std::string>::iterator end = vecFile.end();

	for (;it != end; it++)
	{
		//根据文件ID获取文件名和路径
		char szFileNm[PATH_MAX] = {0};
		char szFilePath[PATH_MAX] = {0};
		int iRet = CDbOper::Instance()->GetFileInfo(it->c_str(), szFileNm, szFilePath);

		if (0 != iRet)
		{
			continue;
		}

		string strCookie;
		TransAtom atom;
		/*strcpy(atom.szFileNm, szFileNm);
		strcpy(atom.szFilePath, szFilePath);
		strcpy(atom.szSvrUrl, strSvrUrl.c_str());
		atom.bCompress = (bool)atoi(strCompress.c_str()) ;
		*/
		if (!GetCookie(strCookie))
		{
			return FALSE;
		}
		strcpy(atom.szCookie, strCookie.c_str());
		
		strcpy(atom.szFileNm, "1.mp4");
		strcpy(atom.szFilePath, "F:\\");
		strcpy(atom.szSvrUrl, "http://192.168.51.40:8080/MyFileUpDownServer/upload");
		atom.bCompress = false ;

		m_pUpload->AddTransferWrok(atom);
	}

	m_pUpload->Start();

	pthread_t thrd;
	pthread_create(&thrd, NULL, upProgress, this);

	return TRUE;
}

void* CPlugin::upProgress(void* arg)
{
	CPlugin *pPlugin = (CPlugin*)arg;

	while(1)
	{
		//调用注册的tansferProgress js函数
		std::string progressJs = pPlugin->m_initedPramaMap["upprogress"];
		if (progressJs.length()>0)
		{
			CStrUtils su;
			std::string js;
			unsigned int iTotalSize = 0;
			unsigned int iTransferSize = 0;
			string strFileNm;
			string strMsg;
			char szTotalLen[MAX_STRING_BUF_SIZE] = {0};
			char szFinshedLen[MAX_STRING_BUF_SIZE] = {0};

			std::string jsFunc = su.GetJsFunction(progressJs); //获得js函数名
			if(jsFunc.length()>0)
			{
				if(pPlugin->m_pUpload->IsTransferBegin())
				{
					iTotalSize = pPlugin->m_pUpload->GetTotalSize();
					iTransferSize = pPlugin->m_pUpload->GetCurTransferSize();
					strFileNm = pPlugin->m_pUpload->GetTransferFileNm();
					strMsg = "";

					memset(szTotalLen, 0, MAX_STRING_BUF_SIZE);
					memset(szFinshedLen, 0, MAX_STRING_BUF_SIZE);
					sprintf(szTotalLen, "%d", iTotalSize);
					sprintf(szFinshedLen, "%d", iTransferSize);

					js = "javascript:try{" + jsFunc +"('" + pPlugin->m_initedPramaMap["id"] +"'," + szTotalLen + "," + szFinshedLen + ",'" + strFileNm + "','" + strMsg + "');}catch(e){}";
					NPN_GetURL(pPlugin->m_pNPInstance, js.c_str(), "_self");
				}
				else if(pPlugin->m_pUpload->IsTransferEnd()) 
				{
					break;
				}
				else if (HAD_TRANS == pPlugin->m_pUpload->GetTransferStatus())
				{
					memset(szTotalLen, 0, MAX_STRING_BUF_SIZE);
					memset(szFinshedLen, 0, MAX_STRING_BUF_SIZE);
					strFileNm = pPlugin->m_pUpload->GetTransferFileNm();
					strMsg = "服务器已存在该文件.";
					char szMsg[MAX_STRING_BUF_SIZE] = {0};
					su.G2U(strMsg.c_str(), szMsg);
					js = "javascript:try{" + jsFunc +"('" + pPlugin->m_initedPramaMap["id"] +"',0,0,'" + strFileNm + "','" + szMsg + "');}catch(e){}";
					int iRet = NPN_GetURL(pPlugin->m_pNPInstance, js.c_str(), "_self");
					js = "";
				}
				else if (OPEN_FILE_ERR == pPlugin->m_pUpload->GetTransferStatus())
				{
					memset(szTotalLen, 0, MAX_STRING_BUF_SIZE);
					memset(szFinshedLen, 0, MAX_STRING_BUF_SIZE);
					strFileNm = pPlugin->m_pUpload->GetTransferFileNm();
					strMsg = "打卡本地文件失败.";
					char szMsg[MAX_STRING_BUF_SIZE] = {0};
					su.G2U(strMsg.c_str(), szMsg);
					js = "javascript:try{" + jsFunc +"('" + pPlugin->m_initedPramaMap["id"] +"',0,0,'" + strFileNm + "','" + szMsg + "');}catch(e){}";
					NPN_GetURL(pPlugin->m_pNPInstance, js.c_str(), "_self");
				}

				else if (TRANS_FAILD == pPlugin->m_pUpload->GetTransferStatus())
				{
					memset(szTotalLen, 0, MAX_STRING_BUF_SIZE);
					memset(szFinshedLen, 0, MAX_STRING_BUF_SIZE);
					strFileNm = pPlugin->m_pUpload->GetTransferFileNm();
					strMsg = "传输失败.";
					char szMsg[MAX_STRING_BUF_SIZE] = {0};
					su.G2U(strMsg.c_str(), szMsg);
					js = "javascript:try{" + jsFunc +"('" + pPlugin->m_initedPramaMap["id"] +"',0,0,'" + strFileNm + "','" + szMsg + "');}catch(e){}";
					NPN_GetURL(pPlugin->m_pNPInstance, js.c_str(), "_self");
				}
			}
		}

		Sleep(10);
	}

	return NULL;
}


void* CPlugin::downProgress(void* arg)
{
	CPlugin *pPlugin = (CPlugin*)arg;

	while(1)
	{
		//调用注册的tansferProgress js函数
		std::string progressJs = pPlugin->m_initedPramaMap["downprogress"];
		if (progressJs.length()>0)
		{
			CStrUtils su;
			std::string js;
			unsigned int iTotalSize = 0;
			unsigned int iTransferSize = 0;
			string strFileNm;
			string strMsg;
			char szTotalLen[MAX_STRING_BUF_SIZE] = {0};
			char szFinshedLen[MAX_STRING_BUF_SIZE] = {0};

			std::string jsFunc = su.GetJsFunction(progressJs); //获得js函数名
			if(jsFunc.length()>0)
			{
				if(pPlugin->m_pDownload->IsTransferBegin())
				{
					iTotalSize = pPlugin->m_pDownload->GetTotalSize();
					iTransferSize = pPlugin->m_pDownload->GetCurTransferSize();
					strFileNm = pPlugin->m_pDownload->GetTransferFileNm();
					strMsg = "";

					memset(szTotalLen, 0, MAX_STRING_BUF_SIZE);
					memset(szFinshedLen, 0, MAX_STRING_BUF_SIZE);
					sprintf(szTotalLen, "%d", iTotalSize);
					sprintf(szFinshedLen, "%d", iTransferSize);

					js = "javascript:try{" + jsFunc +"('" + pPlugin->m_initedPramaMap["id"] +"'," + szTotalLen + "," + szFinshedLen + ",'" + strFileNm + "','" + strMsg + "');}catch(e){}";
					NPN_GetURL(pPlugin->m_pNPInstance, js.c_str(), "_self");
				}
				else if(pPlugin->m_pDownload->IsTransferEnd()) 
				{
					break;
				}
				else if (HAD_TRANS == pPlugin->m_pDownload->GetTransferStatus())
				{
					memset(szTotalLen, 0, MAX_STRING_BUF_SIZE);
					memset(szFinshedLen, 0, MAX_STRING_BUF_SIZE);
					strFileNm = pPlugin->m_pDownload->GetTransferFileNm();
					strMsg = "服务器已存在该文件.";
					char szMsg[MAX_STRING_BUF_SIZE] = {0};
					su.G2U(strMsg.c_str(), szMsg);
					js = "javascript:try{" + jsFunc +"('" + pPlugin->m_initedPramaMap["id"] +"',0,0,'" + strFileNm + "','" + szMsg + "');}catch(e){}";
					int iRet = NPN_GetURL(pPlugin->m_pNPInstance, js.c_str(), "_self");
					js = "";
				}
				else if (OPEN_FILE_ERR == pPlugin->m_pDownload->GetTransferStatus())
				{
					memset(szTotalLen, 0, MAX_STRING_BUF_SIZE);
					memset(szFinshedLen, 0, MAX_STRING_BUF_SIZE);
					strFileNm = pPlugin->m_pDownload->GetTransferFileNm();
					strMsg = "打卡本地文件失败.";
					char szMsg[MAX_STRING_BUF_SIZE] = {0};
					su.G2U(strMsg.c_str(), szMsg);
					js = "javascript:try{" + jsFunc +"('" + pPlugin->m_initedPramaMap["id"] +"',0,0,'" + strFileNm + "','" + szMsg + "');}catch(e){}";
					NPN_GetURL(pPlugin->m_pNPInstance, js.c_str(), "_self");
				}

				else if (TRANS_FAILD == pPlugin->m_pDownload->GetTransferStatus())
				{
					memset(szTotalLen, 0, MAX_STRING_BUF_SIZE);
					memset(szFinshedLen, 0, MAX_STRING_BUF_SIZE);
					strFileNm = pPlugin->m_pDownload->GetTransferFileNm();
					strMsg = "传输失败.";
					char szMsg[MAX_STRING_BUF_SIZE] = {0};
					su.G2U(strMsg.c_str(), szMsg);
					js = "javascript:try{" + jsFunc +"('" + pPlugin->m_initedPramaMap["id"] +"',0,0,'" + strFileNm + "','" + szMsg + "');}catch(e){}";
					NPN_GetURL(pPlugin->m_pNPInstance, js.c_str(), "_self");
				}
			}
		}

		Sleep(10);
	}

	return NULL;
}

BOOL CPlugin::Download(const char* pParam)
{
	CStrUtils su;
	JsonParsor parsor;
	std::map<std::string, std::string> mapParam;
	if (!parsor.Parse(pParam, mapParam))
	{
		return FALSE;
	}

	std::string strFileList = mapParam["fileList"];
	std::string strFileDir = mapParam["destFileDir"];
	std::string strSvrUrl = mapParam["svrHost"];
	std::string strCompress = mapParam["isCompress"];

	std::vector<std::string> vecFile;
	if (0 != su.SplitString(strFileList, ",", vecFile))
	{
		return -1;
	}

	m_pDownload = new CDownload();

	std::vector<std::string>::iterator it = vecFile.begin();
	std::vector<std::string>::iterator end = vecFile.end();

	for (;it != end; it++)
	{
		//根据文件ID获取文件名和路径
		char szFileNm[PATH_MAX] = {0};
		char szFilePath[PATH_MAX] = {0};
		int iRet = CDbOper::Instance()->GetFileInfo(it->c_str(), szFileNm, szFilePath);

		if (0 != iRet)
		{
			continue;
		}

		string strCookie;
		TransAtom atom;
		/*strcpy(atom.szFileNm, szFileNm);
		strcpy(atom.szFilePath, strFileDir.c_str());
		strcpy(atom.szSvrUrl, strSvrUrl.c_str());
		atom.bCompress = (bool)atoi(strCompress.c_str()) ;*/
		if (!GetCookie(strCookie))
		{
			return FALSE;
		}
		strcpy(atom.szCookie, strCookie.c_str());
		strcpy(atom.szFileNm, "1.exe");
		strcpy(atom.szFilePath, "E:\\");
		strcpy(atom.szSvrUrl, "http://192.168.51.40:8080/MyFileUpDownServer/http");
		atom.bCompress = false ;

		m_pDownload->AddTransferWrok(atom);
	}

	m_pDownload->Start();

	pthread_t thrd;
	pthread_create(&thrd, NULL, downProgress, this);

	return TRUE;
}

static LRESULT CALLBACK PluginWinProc(HWND, UINT, WPARAM, LPARAM);
static WNDPROC lpOldProc = NULL;

NPBool CPlugin::init(NPWindow* pNPWindow)
{  
	if(pNPWindow != NULL)
	{
		m_hWnd = (HWND)pNPWindow->window;
		if(m_hWnd == NULL)
		{
			m_bInitialized = FALSE;
		}
		else
		{
			m_bInitialized = TRUE;
			// associate window with our CPlugin object so we can access 
			// it in the window procedure
			SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (LONG_PTR)this);
			// subclass window so we can intercept window messages and
			// do our drawing to it
			lpOldProc = SubclassWindow(m_hWnd, (WNDPROC)PluginWinProc);
		}
	}
	else
	{
		m_bInitialized = FALSE;
	}
	return m_bInitialized;
}

NPObject *
CPlugin::GetScriptableObject()
{
	if (!m_pScriptableObject) {
 		m_pScriptableObject =
 			NPN_CreateObject(m_pNPInstance,
 			&objectClass);
	}

	if (m_pScriptableObject) {
		NPN_RetainObject(m_pScriptableObject);
	}

	return m_pScriptableObject;
}

CPluginObject::CPluginObject(NPP npp)
{
	this->m_npp = npp;
	this->NPIdent_upload = NPN_GetStringIdentifier("upload");
	this->NPIdent_download = NPN_GetStringIdentifier("download");
}

CPluginObject::~CPluginObject(void)
{
}

void CPluginObject::deallocate()
{
}
void CPluginObject::invalidate()
{
}

bool CPluginObject::hasMethod(NPIdentifier methodName)
{
	return (methodName==this->NPIdent_upload
		|| methodName==this->NPIdent_download);
}
bool CPluginObject::invokeDefault(const NPVariant *args, uint32_t argCount, NPVariant *result)
{
	return true;
}
bool CPluginObject::invoke(NPIdentifier methodName, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
	CPlugin* plugin = (CPlugin*)this->m_npp->pdata;
	if (plugin==NULL || !plugin->isInitialized())
	{
		BOOLEAN_TO_NPVARIANT(FALSE, *result);
		return true;
	}

	CStrUtils su;
	char szParam[MAX_STRING_BUF_SIZE] = {0};
	if (this->NPIdent_upload == methodName)
	{
		BOOL isOK = FALSE;
		if(NPVARIANT_IS_STRING(args[0]))
		{
			su.GetString(args[0], szParam, MAX_STRING_BUF_SIZE);
			isOK = plugin->Upload(szParam);
		}
		BOOLEAN_TO_NPVARIANT(isOK, *result);
	}
	else if (this->NPIdent_download == methodName)
	{
		BOOL isOK = FALSE;
		if(NPVARIANT_IS_STRING(args[0]))
		{
			su.GetString(args[0], szParam, MAX_STRING_BUF_SIZE);
			isOK = plugin->Download(szParam);
		}
		BOOLEAN_TO_NPVARIANT(isOK, *result);
	}
	
	return true;
}

bool CPluginObject::hasProperty(NPIdentifier propertyName)
{
	return false;
}
bool CPluginObject::getProperty(NPIdentifier propertyName, NPVariant *result)
{
	return false;
}
bool CPluginObject::setProperty(NPIdentifier name,const NPVariant *value)
{
	return true;
}
bool CPluginObject::removeProperty(NPIdentifier name)
{
	return true;
}
bool CPluginObject::enumerate(NPIdentifier **identifier,uint32_t *count)
{
	return false;
}
bool CPluginObject::construct(const NPVariant *args,uint32_t argCount, NPVariant *result)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
NPObject* CPluginObject::_allocate(NPP npp, NPClass* aClass){
	return new CPluginObject(npp);
}
void CPluginObject::_deallocate(NPObject *npobj)
{
	((CPluginObject*)npobj)->deallocate();
	if(npobj)
		delete npobj;
}
void CPluginObject::_invalidate(NPObject *npobj)
{
	((CPluginObject*)npobj)->invalidate();
}
bool CPluginObject::_hasMethod(NPObject* obj, NPIdentifier methodName)
{
	return ((CPluginObject*)obj)->hasMethod(methodName);
}
bool CPluginObject::_invokeDefault(NPObject *obj, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
	return ((CPluginObject*)obj)->invokeDefault(args,argCount,result);
}
bool CPluginObject::_invoke(NPObject* obj, NPIdentifier methodName, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
	return ((CPluginObject*)obj)->invoke(methodName,args,argCount,result);
}
bool CPluginObject::_hasProperty(NPObject *obj, NPIdentifier propertyName)
{
	return ((CPluginObject*)obj)->hasProperty(propertyName);
}
bool CPluginObject::_getProperty(NPObject *obj, NPIdentifier propertyName, NPVariant *result)
{
	return ((CPluginObject*)obj)->getProperty(propertyName,result);
}
bool CPluginObject::_setProperty(NPObject *npobj, NPIdentifier name,const NPVariant *value)
{
	return ((CPluginObject*)npobj)->setProperty(name,value);
}
bool CPluginObject::_removeProperty(NPObject *npobj, NPIdentifier name)
{
	return ((CPluginObject*)npobj)->removeProperty(name);
}
bool CPluginObject::_enumerate(NPObject *npobj, NPIdentifier **identifier,uint32_t *count)
{
	return ((CPluginObject*)npobj)->enumerate(identifier,count);
}
bool CPluginObject::_construct(NPObject *npobj, const NPVariant *args,uint32_t argCount, NPVariant *result)
{
	return ((CPluginObject*)npobj)->construct(args,argCount,result);
}

/**
 * 消息处理函数
 * 画界面、消息处理都在此函数中实现
 *
 */
static LRESULT CALLBACK PluginWinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg) {
    case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		//
		EndPaint(hWnd, &ps);
		return 0;
	}
	case WM_DESTROY:
	{
		//PostQuitMessage(0); //开启后,chrome刷新时，一次失败，一次成功(后期有时间可以找找这原因)
		break;
	}
  }
  return DefWindowProc(hWnd, msg, wParam, lParam);
}
