#include "ThostFtdcMdApi.h"
#include <Windows.h>
#include <qglobal.h>

// protype,__stdcall,__cdecl,__fastcall
typedef CThostFtdcMdApi*(__cdecl* f_CreateMdApi)(const char* pszFlowPath,
    const bool bIsUsingUdp,
    const bool bIsMulticast);
static f_CreateMdApi p_CreateMdApi = nullptr;

typedef const char*(__cdecl* f_GetApiVersion)();
static f_GetApiVersion p_GetApiVersion = nullptr;

static HMODULE s_mod_mdapi = nullptr;

// init
static void LoadMdApi()
{
    if (!s_mod_mdapi) {
        wchar_t buffer[MAX_PATH] = { 0 };
        GetModuleFileNameW(0, buffer, MAX_PATH);
        //PathRemoveFileSpac(buffer);
        //wcscat(buffer, L"thostmduserapi.dll");
        wchar_t *curFile = wcsrchr(buffer,L'\\') + 1;
        curFile[0] = 0;
        wcscat(buffer,L"thostmduserapi.dll");
        s_mod_mdapi = LoadLibraryW(buffer);
    }
    if (s_mod_mdapi) {
        p_CreateMdApi = (f_CreateMdApi)GetProcAddress(
#ifdef Q_OS_WIN64
            s_mod_mdapi, "?CreateFtdcMdApi@CThostFtdcMdApi@@SAPEAV1@PEBD_N1@Z");
#else
            s_mod_mdapi, "?CreateFtdcMdApi@CThostFtdcMdApi@@SAPAV1@PBD_N1@Z");
#endif
        p_GetApiVersion = (f_GetApiVersion)GetProcAddress(
#ifdef Q_OS_WIN64
            s_mod_mdapi, "?GetApiVersion@CThostFtdcMdApi@@SAPEBDXZ");
#else
            s_mod_mdapi, "?GetApiVersion@CThostFtdcMdApi@@SAPBDXZ");
#endif
    }
}

// proxy
CThostFtdcMdApi* CThostFtdcMdApi::CreateFtdcMdApi(const char* pszFlowPath,
    const bool bIsUsingUdp,
    const bool bIsMulticast)
{
    LoadMdApi();

    if (p_CreateMdApi) {
        return p_CreateMdApi(pszFlowPath, bIsUsingUdp, bIsMulticast);
    } else {
        return nullptr;
    }
}

const char* CThostFtdcMdApi::GetApiVersion()
{
    LoadMdApi();

    if (p_GetApiVersion) {
        return p_GetApiVersion();
    } else {
        if(s_mod_mdapi){
            return "< v6.3.6";
        }else{
            return "no thostmduserapi.dll";
        }
    }
}
