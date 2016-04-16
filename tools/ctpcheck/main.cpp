// gcc main.cpp mdapi_proxy.cpp tdapi_proxy.cpp -std=c++11 -lstdc++ -DUNICODE
// -I"c:/vnsdk/ctp/include" -static -o ctpproxy.exe

#include "ThostFtdcMdApi.h"
#include "ThostFtdcTraderApi.h"
#include <iostream>

// test
void mdapi_test()
{
    CThostFtdcMdApi* mdapi = CThostFtdcMdApi::CreateFtdcMdApi();
    if (mdapi) {
        std::cout << "load mdapi ok!" << std::endl;
        std::cout << "mdapi version: " << CThostFtdcMdApi::GetApiVersion()
                  << std::endl;
        mdapi->Release();
        return;
    }
    std::cout << "load mdapi fail!" << std::endl;
}

void tdapi_test()
{
    CThostFtdcTraderApi* tdapi = CThostFtdcTraderApi::CreateFtdcTraderApi();
    if (tdapi) {
        std::cout << "load tdapi ok!" << std::endl;
        std::cout << "tdapi version: " << CThostFtdcTraderApi::GetApiVersion()
                  << std::endl;
        tdapi->Release();
        return;
    }
    std::cout << "load tdapi fail!" << std::endl;
}

// main
int main(int argc, char* argv[])
{
    mdapi_test();
    tdapi_test();
    return 0;
}
