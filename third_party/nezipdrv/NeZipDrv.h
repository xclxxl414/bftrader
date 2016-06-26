#pragma once

#include <Windows.h>
#include <tchar.h>
#include "Stockdrv.h"

#define		 WM_OEM_CLINENT				(WM_APP + 0X168)			//客户端可以自定义接收数据窗口句柄号

//接收数据窗口句柄，消息号，版本号，网际风软件所在IP地址和端口号，回调函数
typedef BOOL (WINAPI *_InitStockDrv)(HWND hWnd, UINT message, OEM_VERSION dataVersion, LPCSTR serverAddr, LPCSTR serverPort, OEM_STOCKDRV_CALLBACK callBack);
typedef BOOL (WINAPI *_AskData)(TCP_DATA_HEAD* pTcpHead);	//向网际风接口程序申请或提供数据

/*
 * NOTE(hege): askServer设置为true，这样就只有一个notify；不要tick要trace可以读取一天的=
klineType = TRACE_KLINE;
AskData("ZJIC07", klineType,false,true,1,true);		//askServer设置为true则向远程服器同步数据，完成后主动推送数据
klineType = MIN1_KLINE;
AskData("ZJIC07", klineType,false,true,1,true);		//askServer设置为true则向远程服器同步数据，完成后主动推送数据
klineType = MIN5_KLINE;
AskData("ZJIC07", klineType,false,true,1,true);		//askServer设置为true则向远程服器同步数据，完成后主动推送数据
klineType = DAY_KLINE;
AskData("ZJIC07", klineType,false,true,1,true);		//askServer设置为true则向远程服器同步数据，完成后主动推送数据
*/

class NeZipDrv
{
public:
	NeZipDrv(void);
	~NeZipDrv(void);
public:
	bool load(const char *dllfile);
    bool askdata(LPCSTR stkLabel, KLINETYPE klineType, bool sameDay = false, bool isSplit = false, char splitLeft = 1, bool askServer = false);
    bool inited() {return !!m_initialized;}
protected:
    HINSTANCE m_stockdrvDll = nullptr;	//动态库模块句柄
	OEM_VERSION	m_dataVersion;
    _InitStockDrv InitStockDrv_ = nullptr;			//初始化入口
    _AskData	AskData_ = nullptr;						//申请或向网际风提供数据
    BOOL m_initialized = FALSE;		//初始化完成
    UINT m_traceCount = 0;	//模拟分笔数不断增加
    char		m_clientPackInfo[56];	//内部识别数据（由用户填写），申请时原样直接返回

	static BOOL OnCallBack(TCP_DATA_HEAD *pTcpHead);

private:
	static void handle_daydata(TCP_DATA_HEAD *pTcpHead);
	static void handle_mindata(TCP_DATA_HEAD *pTcpHead,bool ismin1);

	static void handle_reportdata(TCP_DATA_HEAD *pTcpHead);
	static void handle_tick(TCP_DATA_HEAD *pTcpHead);
};
