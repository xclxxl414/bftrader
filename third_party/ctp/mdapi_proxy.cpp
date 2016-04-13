#include "ThostFtdcMdApi.h"
#include <Windows.h>

// protype,__stdcall,__cdecl,__fastcall
typedef CThostFtdcMdApi*(__cdecl* f_CreateMdApi)(const char* pszFlowPath,
                                                       const bool bIsUsingUdp,
                                                       const bool bIsMulticast);
static f_CreateMdApi p_CreateMdApi = nullptr;

typedef const char*(__cdecl* f_GetApiVersion)();
static f_GetApiVersion p_GetApiVersion = nullptr;

// init
static void LoadMdApi() {
  static HMODULE s_mod_mdapi = nullptr;
  if (!s_mod_mdapi) {
    s_mod_mdapi = LoadLibraryW(L"thostmduserapi.dll");
  }
  if (s_mod_mdapi) {
    p_CreateMdApi = (f_CreateMdApi)GetProcAddress(
        s_mod_mdapi, "?CreateFtdcMdApi@CThostFtdcMdApi@@SAPEAV1@PEBD_N1@Z");

    p_GetApiVersion = (f_GetApiVersion)GetProcAddress(
        s_mod_mdapi, "?GetApiVersion@CThostFtdcMdApi@@SAPEBDXZ");
  }
}

// proxy
CThostFtdcMdApi* CThostFtdcMdApi::CreateFtdcMdApi(const char* pszFlowPath,
                                                  const bool bIsUsingUdp,
                                                  const bool bIsMulticast) {
  LoadMdApi();

  if (p_CreateMdApi) {
    return p_CreateMdApi(pszFlowPath, bIsUsingUdp, bIsMulticast);
  } else {
    return nullptr;
  }
}

const char* CThostFtdcMdApi::GetApiVersion() {
  LoadMdApi();

  if (p_GetApiVersion) {
    return p_GetApiVersion();
  } else {
    return nullptr;
  }
}
