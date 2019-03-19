#!/bin/bash

INPUT_PATH=proto
OUTPUT_PATH=src/elf/net

PROTOC=
if [ ! -n "$ENV_PATH" ]; then
    PROTOC=protoc
else
    PROTOC=$ENV_PATH/bin/protoc
    export PATH=$PATH:$ENV_PATH/bin
fi
${PROTOC} --version
${PROTOC} -I . --cpp_out=$OUTPUT_PATH $INPUT_PATH/*.proto
${PROTOC} -I . --grpc_out=$OUTPUT_PATH --plugin=protoc-gen-grpc=$ENV_PATH/bin/grpc_cpp_plugin $INPUT_PATH/*.proto

find  . -name "*.cc" | awk '{print "mv "$1" "$1"x"}' | sed "s/ccx/cpp/g" | bash
