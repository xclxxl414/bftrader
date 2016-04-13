grpc c++ helloworld

by和哥 2015-2016

准备
======
1 下载grpc的代码
https://github.com/grpc/grpc/blob/master/INSTALL.md
git clone https://github.com/grpc/grpc.git
修改gitmodule里面的borningssl到github.com/google/borningssl.git
cd grpc
git submodule update --init

2 编译x64 release
https://github.com/grpc/grpc/blob/master/vsprojects/README.md
用vs2015打开grpc.sln，修改为MD，编译x64+release

3 安装cmake
http://www.cmake.org/

4 编译protobuf
third_party\protobuf\cmake\README.md
git clone -b release-1.7.0 https://github.com/google/googlemock.git gmock
cd gmock
git clone -b release-1.7.0 https://github.com/google/googletest.git gtest
cd ..\cmake
cmake -G "Visual Studio 14 2015 Win64"
用vs2015打开protobuf.sln，修改为MD，编译x64+release

5 编译grpc c++/plugin插件
用vs2015打开protobuf.slnvsprojects\grpc_protoc_plugins.sln，修改为MD，编译x64+release
grpc_python_plugin.exe
grpc_cpp_plugin.exe
protoc.exe

构造c++ sdk
======
1. 拷贝grpc的include下全部内容到c:\vnsdk\grpc\include
2. 拷贝protobuf的src下google目录到c:\vnsdk\grpc\include
3. 拷贝grpc的lib+bin到c:\vnsdk\grpc\lib+bin
4. 拷贝protobuf的lib+bin到c:\vnsdk\grpc\lib+bin
5. 拷贝zlib的include+lib+bin到c:\vnsdk\grpc\include+lib+bin
vsprojects\packages\grpc.dependencies.zlib.1.2.8.10\build\native\lib\v140\x64\Release\dynamic\stdcall
vsprojects\packages\grpc.dependencies.zlib.redist.1.2.8.10\build\native\bin\v140\x64\Release\dynamic\stdcall

安装python sdk
======
https://pypi.python.org/pypi/grpcio
grpcio-0.13.0-cp27-cp27m-win32.whl
protobuf-3.0.0b2.post2-py2-none-any.whl

构造go sdk
======
go get google.golang.org/grpc
go get github.com/golang/protobuf/protoc-gen-go
拷贝 protoc-gen-go.exe
说明：
1 go和qt的binding，go到qt通过signal，qt到go通过channel，https://github.com/therecipe/qt
2 用metaobject直接调用也可以

你好世界
======

1 编译proto文件
c:\vnsdk\grpc\bin\protoc.exe -I c:\projects\vnxx\vnsdk\grpc\example\protos --grpc_out=. --plugin=protoc-gen-grpc=c:\vnsdk\grpc\bin\grpc_cpp_plugin.exe c:\projects\vnxx\vnsdk\grpc\example\protos\helloworld.proto
c:\vnsdk\grpc\bin\protoc.exe -I c:\projects\vnxx\vnsdk\grpc\example\protos --cpp_out=. c:\projects\vnxx\vnsdk\grpc\example\protos\helloworld.proto

2 写客户端
grpc\examples\cpp\helloworld\greeter_client.cc

3 写服务端
grpc\examples\cpp\helloworld\greeter_server.cc
