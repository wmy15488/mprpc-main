add_executable(callee
  main.cc
  userservice.cc
)

target_link_libraries(callee
  PRIVATE
    mprpc::mprpc
    user_proto
)

target_include_directories(callee
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
)
