
QQ交流群340195342，点击加入：http://jq.qq.com/?_wv=1027&k=2ADNTk3
===============================================================

2015-2016 by 和哥

用python怎么写策略？（可以使用gateway接口，也可以使用cta接口，目前用gateway接口）
这个就要看懂sdk！看懂sdk/api！
进入sdk目录，运行例子，就明白了！

下载sdk
======
下载地址: https://github.com/sunwangme/bftrader/releases
下载地址: http://pan.baidu.com/s/1nvgrNst

安装grpc for python
======
1. 安装python python-2.7.11.msi
2. 安装python库,python_libs目录下
  2.1 pip install six-xxx
  2.2 pip install setuptools-xxx
  2.3 pip install enum34-xxx
  2.4 pip install futures-xxx
  2.5 pip install protobuf-xxx
  2.6 pip install grpcio-xxx

写策略，调试策略  
=======
1. 运行ctpgateway.exe
2. 点击ctpgateway的rpc start
3. 运行 python proxy_demo.py，连接ctpgateway
4. 点击ctpgateway的ctp start
5. 可以看到proxy_demo.py跑起来

备注：
protoc下面是protobuf+grpc的编译器
=======
1. 执行gen.bat，将sdk/api的接口翻译成py/go/c++
2. 检查 python cpp golang目录，可以看到代码

（完）
