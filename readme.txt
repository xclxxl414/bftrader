QQ交流群340195342，点击加入：http://jq.qq.com/?_wv=1027&k=2ADNTk3
======

2015-2016 by 和哥
bftrader/福友量化: 支持手动交易，也支持程序化

手动交易
======
1. 下载bftrader发布包
下载地址: https://github.com/sunwangme/bftrader/releases
下载地址: http://pan.baidu.com/s/1nvgrNst

2. 运行ctpgateway
先ctp/config，注意地址前面加tcp://，其他和vnpy一致


程序化交易
======
用python怎么写策略？（可以使用gateway接口，也可以使用cta接口，目前用gateway接口）
这个就要看懂sdk！看懂sdk/api！
进入sdk目录，运行例子，就明白了！

1. 下载bftrader发布包
下载地址: https://github.com/sunwangme/bftrader/releases
下载地址: http://pan.baidu.com/s/1nvgrNst

2. 安装grpc for python
   2.1 安装python python-2.7.11.msi
   2.2 安装python库,安装包在 sdk/python_libs目录下
      2.2.1 pip install six-xxx
      2.2.2 pip install setuptools-xxx
      2.2.3 pip install enum34-xxx
      2.2.4 pip install futures-xxx
      2.2.5 pip install protobuf-xxx
      2.2.6 pip install grpcio-xxx

3. 写策略，调试策略  
    3.1 运行ctpgateway.exe
    3.2 点击ctpgateway的rpc/start
    3.3 运行python/demo.py，以连接ctpgateway
    3.4 点击ctpgateway的ctp/start
    3.5 可以看到demo.py跑起来啦

从源码编译
======
1. 安装vs2015 社区版，主要一定要选择 web开发工具（不然会有framework找不到的问题），另外加上vc2015的公用工具，就这两项(会自动安装wsdk81)
https://www.visualstudio.com/downloads/download-visual-studio-vs

2. 安装调试器 windbg/x64
http://pan.baidu.com/s/1nvgrNst
tools/dbg_amd64.msi

3. 安装qt-5.6.0 (内置了qtcreator，bftrader使用qtcreator做c++开发环境)，配置好qtcreator，新建对话框试试！
http://download.qt.io/official_releases/qt/5.6/5.6.0/
qt-opensource-windows-x86-msvc2015_64-5.6.0.exe

4. 下载源代码，解压到c:/projects/bftrader目录
https://github.com/sunwangme/bftrader/releases

5. 下载支持库，解压到c:/projects/bftrader/third_party目录
http://pan.baidu.com/s/1nvgrNst
third_party.7z

6. 用qtcreator打开 c:/projects/bftrader/bftrader.pro
编译！！！！

（完）
