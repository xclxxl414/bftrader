编译步骤

by和哥 2015-2016

准备
======
1 下载代码

git clone https://github.com/sunwangme/togo.git

2 编译x86 release/debug
用vs2015打开ta-lib\c\ide\vs2015\dll_proj\tablib.sln，
编译x86+release/debug

构造c++ sdk
======
1. 拷贝ta-lib\c\ide\vs2015\dll_proj\talib\ztalib.h到C:\projects\bftrader\third_party\talib\include
2. 拷贝ta-lib\c\bin的dll pdf到C:\projects\bftrader\third_party\talib\bin
3. 拷贝ta-lib\c\bin的lib到C:\projects\bftrader\third_party\talib\lib
