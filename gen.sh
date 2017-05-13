#!/bin/bash

INPUT_PATH=proto
OUTPUT_PATH=src/elf/net

protoc -I . --cpp_out=$OUTPUT_PATH $INPUT_PATH/*.proto
protoc -I . --grpc_out=$OUTPUT_PATH --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` $INPUT_PATH/*.proto

find  . -name "*.cc" | awk '{print "mv "$1" "$1"x"}' | sed "s/ccx/cpp/g" | bash
