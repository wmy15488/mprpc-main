set(EXAMPLE_PROTO_FILES
  user.proto
)

add_library(user_proto STATIC)

target_sources(user_proto
  PRIVATE
    ${EXAMPLE_PROTO_FILES}
)

protobuf_generate(
  TARGET user_proto
  LANGUAGE cpp
  PROTOC_OUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_include_directories(user_proto
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(user_proto
  PUBLIC
    protobuf::libprotobuf
)

add_subdirectory(caller)
add_subdirectory(callee)
