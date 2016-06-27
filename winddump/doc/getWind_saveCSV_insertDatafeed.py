# coding=utf-8
from WindPy import w
w.start()
import os
import datetime as dt
import numpy as np
import pandas as pd

import copy
import re



bf_path = 'G:\\bftrader'
wind_path = os.path.join(bf_path,'wind')
data_path = os.path.join(wind_path,'data')
set_path = os.path.join(data_path, 'set')
ind_wi_path = os.path.join(data_path, 'ind_wi')
ind_wi_path_D01 = os.path.join(ind_wi_path, 'D01')
ind_wi_path_M01 = os.path.join(ind_wi_path, 'M01')
ind_wi_path_M03 = os.path.join(ind_wi_path, 'M03')
ind_wi_path_M15 = os.path.join(ind_wi_path, 'M15')
ind_wi_path_M60 = os.path.join(ind_wi_path, 'M60')
ind_path = os.path.join(data_path, 'ind')
ind_path_D01 = os.path.join(ind_path, 'D01')
ind_path_M01 = os.path.join(ind_path, 'M01')
ind_path_M03 = os.path.join(ind_path, 'M03')
ind_path_M15 = os.path.join(ind_path, 'M15')
ind_path_M60 = os.path.join(ind_path, 'M60')
dat_path = os.path.join(data_path, 'dat')
dat_path_D01 = os.path.join(dat_path, 'D01')
dat_path_M01 = os.path.join(dat_path, 'M01')
dat_path_M03 = os.path.join(dat_path, 'M03')
dat_path_M15 = os.path.join(dat_path, 'M15')
dat_path_M60 = os.path.join(dat_path, 'M60')


if os.path.exists(wind_path)==False:
    os.mkdir(wind_path)
if os.path.exists(data_path)==False:
    os.mkdir(data_path)
    os.mkdir(set_path)
    os.mkdir(ind_wi_path)
    os.mkdir(ind_wi_path_D01)
    os.mkdir(ind_wi_path_M01)
    os.mkdir(ind_wi_path_M03)
    os.mkdir(ind_wi_path_M15)
    os.mkdir(ind_wi_path_M60)
    os.mkdir(ind_path)
    os.mkdir(ind_path_D01)
    os.mkdir(ind_path_M01)
    os.mkdir(ind_path_M03)
    os.mkdir(ind_path_M15)
    os.mkdir(ind_path_M60)
    os.mkdir(dat_path)
    os.mkdir(dat_path_D01)
    os.mkdir(dat_path_M01)
    os.mkdir(dat_path_M03)
    os.mkdir(dat_path_M15)
    os.mkdir(dat_path_M60)

    FirstRun = 1
else:
    FirstRun = 0

dt_now = dt.datetime.now()
if FirstRun==1:
    dt_start_day = dt_now - dt.timedelta(days=365)
else:
    dt_start_day = dt_now - dt.timedelta(days=7)
dt_now_str = dt_now.strftime('%Y-%m-%d')

# IC 单独考虑

C_list = ['SR.CZC','RB.SHF','PP.DCE','M.DCE']
WI_list = ['SRFI.WI','RBFI.WI','PPFI.WI','MFI.WI']
# # wind的商品指数
ind_dict = {}
for c in C_list:
    data = w.wsd(c, 'open,high,low,close,volume,oi,trade_hiscode', dt_start_day, dt_now)
    data = pd.DataFrame(np.transpose(data.Data),columns=data.Fields,index=data.Times)
    data = data.resample('D').last().dropna()
    ind_dict[c] = data
    data.to_csv(os.path.join(ind_wi_path_D01,c + '.csv'), index=True,header=True)
# # wind的商品指数的合约集合
set_dict = {}
for c in WI_list:
    data = w.wsd(c, 'open,high,low,close,volume', dt_start_day, dt_now)
    data = pd.DataFrame(np.transpose(data.Data),columns=data.Fields,index=data.Times)
    data = data.resample('D').last().dropna()
    data.to_csv(os.path.join(ind_wi_path_D01,c + '.csv'), index=True,header=True)
    data = w.wset('sectorconstituent','date=' + dt_now_str + ';windcode=' + c)
    data = pd.DataFrame(np.transpose(data.Data), columns=data.Fields)
    del data['date']
    set_dict[c] = data
    data.to_csv(os.path.join(set_path, c + '.csv'), index=True, header=True, encoding='gbk')
# # 下载各个合约日线数据，并计算指数
for s in set_dict.keys():
    ind = {}
    ind_df = pd.DataFrame()
    ind_df_open = pd.DataFrame()
    ind_df_high = pd.DataFrame()
    ind_df_low = pd.DataFrame()
    ind_df_close = pd.DataFrame()
    ind_df_volume = pd.DataFrame()
    ind_df_oi = pd.DataFrame()
    for c in set_dict[s]['wind_code']:
        data = w.wsd(c, 'open,high,low,close,volume,oi,trade_hiscode', dt_start_day, dt_now)
        data = pd.DataFrame(np.transpose(data.Data), columns=data.Fields, index=data.Times)
        data = data.resample('D').last().dropna()
        ind[c] = data
        data.to_csv(os.path.join(dat_path_D01, c + '.csv'), index=True, header=True)
        ind_df_open[c] =  pd.to_numeric(data['OPEN'])
        ind_df_high[c] =  pd.to_numeric(data['HIGH'])
        ind_df_low[c] =   pd.to_numeric(data['LOW'])
        ind_df_close[c] = pd.to_numeric(data['CLOSE'])
        ind_df_volume[c] =pd.to_numeric(data['VOLUME'])
        ind_df_oi[c] =    pd.to_numeric(data['OI'])
    ind_df['OPEN' ] = ((ind_df_open * ind_df_oi).sum(axis=1) / (ind_df_oi).sum(axis=1)).round(3)
    ind_df['HIGH' ] = ((ind_df_high * ind_df_oi).sum(axis=1) / (ind_df_oi).sum(axis=1)).round(3)
    ind_df['LOW'  ] = ((ind_df_low * ind_df_oi).sum(axis=1) / (ind_df_oi).sum(axis=1)).round(3)
    ind_df['CLOSE'] = ((ind_df_close * ind_df_oi).sum(axis=1) / (ind_df_oi).sum(axis=1)).round(3)
    ind_df['VOLUME'] =(ind_df_volume).sum(axis=1)
    ind_df['OI'] = (ind_df_oi).sum(axis=1)
    ind_df.to_csv(os.path.join(ind_path_D01, s[:-5] + '.csv'), index=True, header=True, encoding='gbk')






