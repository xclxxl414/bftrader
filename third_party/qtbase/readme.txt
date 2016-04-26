by和哥 2015-2016

准备
======
1 安装vs2015 社区版，主要一定要选择 web开发工具（不然会有framework找不到的问题），另外加上vc2015的公用工具，就这两项(会自动安装wsdk81)
https://www.visualstudio.com/downloads/download-visual-studio-vs

2. 安装调试器 windbg/x64
http://pan.baidu.com/s/1nvgrNst

3 安装python27-x86 和activeperl512-x86
http://www.activestate.com/activeperl/
http://www.activestate.com/activepython/
http://pan.baidu.com/s/1nvgrNst

4 下载qtbase和qttools代码
http://download.qt.io/official_releases/qt/5.6/5.6.0/submodules/
http://pan.baidu.com/s/1nvgrNst

qtbase：x64 shared+debug+release编译
======
configure -prefix "C:\qtbase" -confirm-license -opensource -debug-and-release -force-debug-info -shared -accessibility -nomake tests -nomake examples -no-compile-examples -c++11 -ltcg -qt-sql-sqlite -plugin-sql-sqlite -no-freetype -no-opengl -no-qml-debug -no-icu -no-angle -qt-zlib -qt-libpng -qt-libjpeg -no-openssl -no-dbus -no-audio-backend -no-wmf-backend -no-style-fusion -mp -platform win32-msvc2015
nmake
nmake install

qttools：文档工具（qdoc），部署工具（windeployqt）
======
先打补丁，拷贝patch\qttools\src目录到qttools\src目录覆盖
然后：
qmake
nmake
nmake install

补丁说明：
qdoc里面混了一些qml相关的，都注释了就行了, 修改的文件：
C:\projects\qttools-opensource-src-5.6.0\src\qdoc\qdoc.pro
C:\projects\qttools-opensource-src-5.6.0\src\qdoc\main.cpp

assist\qhelpgenerator静态编译情况下没有自动连接插件qsqlite
C:\projects\qttools-opensource-src-5.6.0\src\assistant\qhelpgenerator\qhelpgenerator.pro

docs：文档！
======
cd qtbase
nmake docs
nmake install_docs

cd qttools
nmake docs
nmake install_docs
