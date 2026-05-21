set(MPRPC_PROTO_FILES
  mprpc.proto
)

add_library(mprpc_proto STATIC)

target_sources(mprpc_proto
  PRIVATE
    ${MPRPC_PROTO_FILES}
)

protobuf_generate(
  TARGET mprpc_proto
  LANGUAGE cpp
  PROTOC_OUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_include_directories(mprpc_proto
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(mprpc_proto
  PUBLIC
    protobuf::libprotobuf
)

add_library(mprpc_proto_dependencies INTERFACE)

target_link_libraries(mprpc_proto_dependencies
  INTERFACE
    mprpc_proto
)
