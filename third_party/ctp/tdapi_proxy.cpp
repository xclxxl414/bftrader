#include "ThostFtdcTraderApi.h"
#include <Windows.h>

// protype,__stdcall,__cdecl,__fastcall
typedef CThostFtdcTraderApi*(__cdecl* f_CreateTraderApi)(
    const char* pszFlowPath);
static f_CreateTraderApi p_CreateTraderApi = nullptr;

typedef const char*(__cdecl* f_GetApiVersion)();
static f_GetApiVersion p_GetApiVersion = nullptr;

// init
static void LoadTdApi() {
  static HMODULE s_mod_tdapi = nullptr;
  if (!s_mod_tdapi) {
    s_mod_tdapi = LoadLibraryW(L"thosttraderapi.dll");
  }
  if (s_mod_tdapi) {
    p_CreateTraderApi = (f_CreateTraderApi)GetProcAddress(
        s_mod_tdapi, "?CreateFtdcTraderApi@CThostFtdcTraderApi@@SAPEAV1@PEBD@Z");

    p_GetApiVersion = (f_GetApiVersion)GetProcAddress(
        s_mod_tdapi, "?GetApiVersion@CThostFtdcTraderApi@@SAPEBDXZ");
  }
}

// proxy
CThostFtdcTraderApi* CThostFtdcTraderApi::CreateFtdcTraderApi(
    const char* pszFlowPath) {
  LoadTdApi();

  if (p_CreateTraderApi) {
    return p_CreateTraderApi(pszFlowPath);
  } else {
    return nullptr;
  }
}

const char* CThostFtdcTraderApi::GetApiVersion() {
  LoadTdApi();

  if (p_GetApiVersion) {
    return p_GetApiVersion();
  } else {
    return nullptr;
  }
}
