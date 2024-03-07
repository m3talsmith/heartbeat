cmake_minimum_required(VERSION 3.22)

project(client)

set(CMAKE_CXX_STANDARD 20)

# Abseil dependencies
find_package(abseil)

# Protobuf dependencies
set(protobuf_MODULE_COMPATIBLE TRUE)

find_package(protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(Threads)

# Protobuf variables
set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
set(_REFLECTION gRPC::grpc++_reflection)
if(CMAKE_CROSSCOMPILING)
    find_program(_PROTOBUF_PROTOC protoc)
else()
    set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
endif()

set(_GRPC_GRPCPP gRPC::grpc++)
if(CMAKE_CROSSCOMPILING)
    find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
else()
    set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)
endif()

get_filename_component(heartbeat_proto "../proto/heartbeat.proto" ABSOLUTE)
get_filename_component(heartbeat_proto_path "${heartbeat_proto}" PATH)

set(heartbeat_proto_sources "${CMAKE_CURRENT_BINARY_DIR}/heartbeat.pb.cc")
set(heartbeat_proto_headers "${CMAKE_CURRENT_BINARY_DIR}/heartbeat.pb.h")
set(heartbeat_grpc_sources "${CMAKE_CURRENT_BINARY_DIR}/heartbeat.grpc.pb.cc")
set(heartbeat_grpc_headers "${CMAKE_CURRENT_BINARY_DIR}/heartbeat.grpc.pb.h")

add_custom_command(
        OUTPUT "${heartbeat_proto_sources}" "${heartbeat_proto_headers}" "${heartbeat_grpc_sources}" "${heartbeat_grpc_headers}"
        COMMAND ${_PROTOBUF_PROTOC}
        ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}" --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${heartbeat_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${heartbeat_proto}"
        DEPENDS "${heartbeat_proto}"
)

# Protobuf and binary generation
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

add_library(heartbeat_grpc_proto
        ${heartbeat_proto_sources}
        ${heartbeat_proto_headers}
        ${heartbeat_grpc_sources}
        ${heartbeat_grpc_headers}
)

target_link_libraries(heartbeat_grpc_proto
        ${_PROTOBUF_LIBPROTOBUF}
        ${_REFLECTION}
        ${_GRPC_GRPCPP}
)

add_executable(client client.cc)

target_link_libraries(client
        heartbeat_grpc_proto
        absl::flags
        absl::flags_parse
        ${_PROTOBUF_LIBPROTOBUF}
        ${_REFLECTION}
        ${_GRPC_GRPCPP}
)