protoc -I . --cpp_out=cpp --grpc_out=cpp --plugin=protoc-gen-grpc=../third_party/grpc/bin/grpc_cpp_plugin.exe ./bfgateway.proto --proto_path=../third_party/grpc/include
protoc -I . --cpp_out=cpp --grpc_out=cpp --plugin=protoc-gen-grpc=../third_party/grpc/bin/grpc_cpp_plugin.exe ./bfdatafeed.proto --proto_path=../third_party/grpc/include
protoc -I . --cpp_out=cpp --grpc_out=cpp --plugin=protoc-gen-grpc=../third_party/grpc/bin/grpc_cpp_plugin.exe ./bfkv.proto --proto_path=../third_party/grpc/include
