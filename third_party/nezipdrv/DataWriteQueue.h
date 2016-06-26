#pragma once
#include "decimal.h"

#include <string>
#include <vector>
#include <map>
#include <list>
#include <mutex>

class KLineData
{
public:
    std::string mkt;
	std::string code;
    //int64_t rq;
    uint32_t time;
    //dec::decimal4 openpx;
    //dec::decimal4 highpx;
    //dec::decimal4 lowpx;
    //dec::decimal4 closepx;
    float open;
    float high;
    float low;
    float close;

    float volume;
    float amount;
    //int32_t vol;
    //dec::decimal4 money;
    //dec::decimal4 jspx;
    //int32_t pos;
};

class TickData
{
public:
	std::string mkt;
	std::string code;
    //int64_t rq;
    uint32_t time;
    //dec::decimal4 lastclose;
    //dec::decimal4 openpx;
    //dec::decimal4 newpx;
    //int32_t vol;
    //dec::decimal4 money;//期货这里是持仓，股票是成交量=
    float close;
    float avPrice;

    float volume;
    float amount;

    float   pricebuy[3];  //申买价1,2,3
    float   volbuy[3];	  //申买量1,2,3
    float   pricesell[3]; //申卖价1,2,3
    float   volsell[3];	  //申卖量1,2,3

    //dec::decimal4 buypx[3];
    //dec::decimal4 buyvol[3];
    //dec::decimal4 sellpx[3];
    //dec::decimal4 sellvol[3];
};

/*
 * NOTE(hege): 外部弄个线程+定时器，每1秒或者100毫秒，读一次数据然后写=
 */
class DataWriteQueue
{
public:
	void merge_daydata(std::list<KLineData> &kd)
	{
        std::unique_lock<std::mutex> lock(mtx);
		daydatalst.splice(daydatalst.end(),kd);
	}

    void merge_min1data(std::list<KLineData> &kd)
	{
        std::unique_lock<std::mutex> lock(mtx);
        min1datalst.splice(min1datalst.end(), kd);
	}

    void merge_min5data(std::list<KLineData> &kd)
	{
        std::unique_lock<std::mutex> lock(mtx);
        min5datalst.splice(min5datalst.end(), kd);
	}

    void merge_tickdata(std::list<TickData> &kd)
	{
        std::unique_lock<std::mutex> lock(mtx);
        tickdatalst.splice(tickdatalst.end(), kd);
	}

    int getdatasize(int &daysize,int &min1size,int &min5size,int &ticksize)
	{
        std::unique_lock<std::mutex> lock(mtx);
        daysize=daydatalst.size();
		min1size = min1datalst.size();
		min5size = min5datalst.size();
		ticksize = tickdatalst.size();
		return 0;
	}

	void get_daydata(std::list<KLineData> &kd)
	{
        get_klinedata(kd, daydatalst);
	}

	void get_min1data(std::list<KLineData> &kd)
	{
        get_klinedata(kd, min1datalst);
	}

	void get_min5data(std::list<KLineData> &kd)
	{
        get_klinedata(kd, min5datalst);
	}

    void get_tickdata(std::list<TickData> &td)
	{
        std::unique_lock<std::mutex> lock(mtx);
        if (tickdatalst.size() <= 8192)
		{
            td.swap(tickdatalst);
		}
		else
		{
			std::list<TickData>::iterator it = tickdatalst.begin();
			std::list<TickData>::iterator it2 = it;
			for (int i = 0; i < 8192; i++)
				it2++;
            td.splice(td.end(), tickdatalst, it, it2);
		}
	}

private:
    void get_klinedata(std::list<KLineData> &kd, std::list<KLineData> &kdlst)
	{
        std::unique_lock<std::mutex> lock(mtx);
        if (kdlst.size() <= 8192)
		{
			kd.swap(kdlst);
		}
		else
		{
			std::list<KLineData>::iterator it = kdlst.begin();
			std::list<KLineData>::iterator it2 = it;
			for (int i = 0; i < 8192; i++)
				it2++;
			kd.splice(kd.end(), kdlst, it, it2);
		}
	}

private:
    std::list<TickData> tickdatalst;
    std::list<KLineData> min1datalst;
    std::list<KLineData> min5datalst;
    std::list<KLineData> daydatalst;

    std::mutex mtx;
};

extern DataWriteQueue gDataWriteQueue;
