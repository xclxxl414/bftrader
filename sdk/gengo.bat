protoc -I . --go_out=plugins=grpc:. ./bfgateway.proto --proto_path=../third_party/grpc/include 
protoc -I . --go_out=plugins=grpc:. ./bfdatafeed.proto --proto_path=../third_party/grpc/include 
protoc -I . --go_out=plugins=grpc:. ./bfkv.proto --proto_path=../third_party/grpc/include 
