by和哥 2015-2016

编译器和相关约定
======
vs2015社区版.update1 x64/64位 shared/动态链接
windbg x64/64位

用到的库和框架：
======
1. ctp 期货内盘交易接口
2. grpc 协议定义&消息序列化&网络通信框架
3. qtbase 界面和应用开发框架 （可直接使用qt官网版本，这里提供手动编译步骤，体积小很多）
4. breakpad leveldb mhook qcustomplot等知名小库

目录结构：请将bin目录加入系统PATH
======
third_party\grpc\bin
third_party\grpc\include
third_party\grpc\lib

third_party\ctp\bin
third_party\ctp\include
third_party\ctp\lib

......

下载地址：
======
http://pan.baidu.com/s/1nvgrNst