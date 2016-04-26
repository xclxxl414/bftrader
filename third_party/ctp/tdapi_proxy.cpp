#include "ThostFtdcTraderApi.h"
#include <Windows.h>
#include <qglobal.h>

// protype,__stdcall,__cdecl,__fastcall
typedef CThostFtdcTraderApi*(__cdecl* f_CreateTraderApi)(
    const char* pszFlowPath);
static f_CreateTraderApi p_CreateTraderApi = nullptr;

typedef const char*(__cdecl* f_GetApiVersion)();
static f_GetApiVersion p_GetApiVersion = nullptr;

static HMODULE s_mod_tdapi = nullptr;

// init
static void LoadTdApi()
{
    if (!s_mod_tdapi) {
        wchar_t buffer[MAX_PATH] = { 0 };
        GetModuleFileNameW(0, buffer, MAX_PATH);
        //PathRemoveFileSpac(buffer);
        //wcscat(buffer, L"thosttraderapi.dll");
        wchar_t *curFile = wcsrchr(buffer,L'\\') + 1;
        curFile[0] = 0;
        wcscat(buffer,L"thosttraderapi.dll");
        s_mod_tdapi = LoadLibraryW(buffer);
    }
    if (s_mod_tdapi) {
        p_CreateTraderApi = (f_CreateTraderApi)GetProcAddress(
#ifdef Q_OS_WIN64
            s_mod_tdapi, "?CreateFtdcTraderApi@CThostFtdcTraderApi@@SAPEAV1@PEBD@Z");
#else
            s_mod_tdapi, "?CreateFtdcTraderApi@CThostFtdcTraderApi@@SAPAV1@PBD@Z");
#endif

        p_GetApiVersion = (f_GetApiVersion)GetProcAddress(
#ifdef Q_OS_WIN64
            s_mod_tdapi, "?GetApiVersion@CThostFtdcTraderApi@@SAPEBDXZ");
#else
            s_mod_tdapi, "?GetApiVersion@CThostFtdcTraderApi@@SAPBDXZ");
#endif
    }
}

// proxy
CThostFtdcTraderApi* CThostFtdcTraderApi::CreateFtdcTraderApi(
    const char* pszFlowPath)
{
    LoadTdApi();

    if (p_CreateTraderApi) {
        return p_CreateTraderApi(pszFlowPath);
    } else {
        return nullptr;
    }
}

const char* CThostFtdcTraderApi::GetApiVersion()
{
    LoadTdApi();

    if (p_GetApiVersion) {
        return p_GetApiVersion();
    } else {
        if (s_mod_tdapi){
            return "< v6.3.6";
        }
        else{
            return "no thosttraderapi.dll";
        }
    }
}
