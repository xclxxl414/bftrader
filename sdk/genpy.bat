protoc -I . --python_out=python --grpc_out=python --plugin=protoc-gen-grpc=../third_party/grpc/bin/grpc_python_plugin.exe ./bfgateway.proto --proto_path=../third_party/grpc/include
protoc -I . --python_out=python --grpc_out=python --plugin=protoc-gen-grpc=../third_party/grpc/bin/grpc_python_plugin.exe ./bfdatafeed.proto --proto_path=../third_party/grpc/include
protoc -I . --python_out=python --grpc_out=python --plugin=protoc-gen-grpc=../third_party/grpc/bin/grpc_python_plugin.exe ./bfkv.proto --proto_path=../third_party/grpc/include
