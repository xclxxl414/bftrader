protoc -I ./protos --python_out=python --grpc_out=python --plugin=protoc-gen-grpc=c:/vnsdk/grpc/bin/grpc_python_plugin.exe ./protos/helloworld.proto
protoc -I ./protos --cpp_out=cpp --grpc_out=cpp --plugin=protoc-gen-grpc=c:/vnsdk/grpc/bin/grpc_cpp_plugin.exe ./protos/helloworld.proto
protoc -I ./protos --go_out=plugins=grpc:golang ./protos/helloworld.proto
