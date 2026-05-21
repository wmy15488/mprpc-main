add_executable(caller
  main.cc
)

target_link_libraries(caller
  PRIVATE
    mprpc::mprpc
    user_proto
)
