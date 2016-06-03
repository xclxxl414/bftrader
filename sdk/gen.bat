protoc -I ./api --python_out=python/bftrader --grpc_out=python/bftrader --plugin=protoc-gen-grpc=../third_party/grpc/bin/grpc_python_plugin.exe ./api/bfgateway.proto --proto_path=../third_party/grpc/include
protoc -I ./api --cpp_out=cpp/bftrader --grpc_out=cpp/bftrader --plugin=protoc-gen-grpc=../third_party/grpc/bin/grpc_cpp_plugin.exe ./api/bfgateway.proto --proto_path=../third_party/grpc/include
protoc -I ./api --go_out=plugins=grpc:golang/bftrader ./api/bfgateway.proto --proto_path=../third_party/grpc/include

protoc -I ./api --python_out=python/bftrader --grpc_out=python/bftrader --plugin=protoc-gen-grpc=../third_party/grpc/bin/grpc_python_plugin.exe ./api/bfdatafeed.proto --proto_path=../third_party/grpc/include
protoc -I ./api --cpp_out=cpp/bftrader --grpc_out=cpp/bftrader --plugin=protoc-gen-grpc=../third_party/grpc/bin/grpc_cpp_plugin.exe ./api/bfdatafeed.proto --proto_path=../third_party/grpc/include
protoc -I ./api --go_out=plugins=grpc:golang/bftrader ./api/bfdatafeed.proto --proto_path=../third_party/grpc/include

protoc -I ./api --python_out=python/bftrader --grpc_out=python/bftrader --plugin=protoc-gen-grpc=../third_party/grpc/bin/grpc_python_plugin.exe ./api/bfkv.proto --proto_path=../third_party/grpc/include
protoc -I ./api --cpp_out=cpp/bftrader --grpc_out=cpp/bftrader --plugin=protoc-gen-grpc=../third_party/grpc/bin/grpc_cpp_plugin.exe ./api/bfkv.proto --proto_path=../third_party/grpc/include
protoc -I ./api --go_out=plugins=grpc:golang/bftrader ./api/bfkv.proto --proto_path=../third_party/grpc/include

protoc -I ./api --python_out=python/bftrader --grpc_out=python/bftrader --plugin=protoc-gen-grpc=../third_party/grpc/bin/grpc_python_plugin.exe ./api/bfcta.proto --proto_path=../third_party/grpc/include
protoc -I ./api --cpp_out=cpp/bftrader --grpc_out=cpp/bftrader --plugin=protoc-gen-grpc=../third_party/grpc/bin/grpc_cpp_plugin.exe ./api/bfcta.proto --proto_path=../third_party/grpc/include
protoc -I ./api --go_out=plugins=grpc:golang/bftrader ./api/bfcta.proto --proto_path=../third_party/grpc/include
