#pragma once
#include <windows.h>
#include <windowsx.h>
#include <string>
#include <map>
#include "pluginbase.h"
#include "Transfer.h"

class CPlugin :
	public nsPluginInstanceBase
{
private:
	HWND m_hWnd; //窗口句柄
	NPP m_pNPInstance;
	NPBool m_bInitialized;
	std::map<std::string,std::string> m_initedPramaMap; //启动参数
public:
	CPlugin(nsPluginCreateData* data);
	~CPlugin();

	NPBool init(NPWindow* pNPWindow);
	void shut()  {  m_bInitialized = FALSE;  }
	NPBool isInitialized()  {  return m_bInitialized;  }

	NPObject *GetScriptableObject();
	NPObject *m_pScriptableObject;
	BOOL GetCookie(std::string &cookies);
	NPError GetCookie(const char *pUrl, char **pValue, uint32_t *uiLen);

	HWND GetHWND();
	BOOL Upload(const char* pParam);
	BOOL Download(const char* pParam);
	static void* upProgress(void* arg);
	static void* downProgress(void* arg);

	std::string m_app_folder;

	CTransfer* m_pUpload;
	CTransfer* m_pDownload;
};


class CPluginObject : public NPObject
{
public:
	CPluginObject(NPP);
	~CPluginObject(void);
	void deallocate();
	void invalidate();
	bool hasMethod(NPIdentifier methodName);
	bool invokeDefault(const NPVariant *args, uint32_t argCount, NPVariant *result);
	bool invoke(NPIdentifier methodName, const NPVariant *args, uint32_t argCount, NPVariant *result);
	bool hasProperty(NPIdentifier propertyName);
	bool getProperty(NPIdentifier propertyName, NPVariant *result);
	bool setProperty(NPIdentifier name,const NPVariant *value);
	bool removeProperty(NPIdentifier name);
	bool enumerate(NPIdentifier **identifier,uint32_t *count);
	bool construct(const NPVariant *args,uint32_t argCount, NPVariant *result);
public:
	static NPObject* _allocate(NPP npp,NPClass* aClass);
	static void _deallocate(NPObject *npobj);
	static void _invalidate(NPObject *npobj);
	static bool _hasMethod(NPObject* obj, NPIdentifier methodName);
	static bool _invokeDefault(NPObject *obj, const NPVariant *args, uint32_t argCount, NPVariant *result);
	static bool _invoke(NPObject* obj, NPIdentifier methodName, const NPVariant *args, uint32_t argCount, NPVariant *result);
	static bool _hasProperty(NPObject *obj, NPIdentifier propertyName);
	static bool _getProperty(NPObject *obj, NPIdentifier propertyName, NPVariant *result);
	static bool _setProperty(NPObject *npobj, NPIdentifier name,const NPVariant *value);
	static bool _removeProperty(NPObject *npobj, NPIdentifier name);
	static bool _enumerate(NPObject *npobj, NPIdentifier **identifier,uint32_t *count);
	static bool _construct(NPObject *npobj, const NPVariant *args,uint32_t argCount, NPVariant *result);
private:
	NPP m_npp;
public:
	NPIdentifier NPIdent_upload;
	NPIdentifier NPIdent_download;
};

#ifndef __object_class
#define __object_class
static NPClass objectClass = {	
	NP_CLASS_STRUCT_VERSION,
	CPluginObject::_allocate,
	CPluginObject::_deallocate,
	CPluginObject::_invalidate,
	CPluginObject::_hasMethod,
	CPluginObject::_invoke,
	CPluginObject::_invokeDefault,
	CPluginObject::_hasProperty,
	CPluginObject::_getProperty,
	CPluginObject::_setProperty,
	CPluginObject::_removeProperty,
	CPluginObject::_enumerate,
	CPluginObject::_construct
};
#endif
