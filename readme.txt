QQ交流群340195342，点击加入：http://jq.qq.com/?_wv=1027&k=2ADNTk3
======

2015-2016 by 和哥
bftrader/福友量化: 支持手动交易，也支持程序化

从源码编译
======
照着做，很快能搞定。
doc\bftrader编译步骤-文字版.txt
doc\bftrader编译步骤-图文版.docx

手动交易
======
1. 下载bftrader发布包
下载地址: https://github.com/sunwangme/bftrader/releases
下载地址: http://pan.baidu.com/s/1nvgrNst

2. 运行allinone
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
    3.1 运行allinone.exe
    3.2 点击allinone的rpc/start
    3.3 运行python/demo.py，以连接allinone
    3.4 点击allinone的ctp/start
    3.5 可以看到demo.py跑起来啦


（完）
