add_library(mprpc STATIC
  logger.cc
  mprpc.cc
  mprpcapplication.cc
  mprpcchannel.cc
  mprpcconfig.cc
  mprpccontroller.cc
  rpcprovider.cc
  zkclient.cc
)

add_library(mprpc::mprpc ALIAS mprpc)

target_include_directories(mprpc
  PUBLIC
    "${CMAKE_SOURCE_DIR}/src/include"
)

target_link_libraries(mprpc
  PUBLIC
    mprpc_proto
    mymuduo::mymuduo
    protobuf::libprotobuf
    zookeeper_mt
)

add_library(mprpc_dependencies INTERFACE)

target_link_libraries(mprpc_dependencies
  INTERFACE
    mprpc::mprpc
)

if(TARGET mymuduo::mymuduo)
  target_link_libraries(mprpc_dependencies INTERFACE mymuduo::mymuduo)
endif()

if(TARGET mprpc_proto_dependencies)
  target_link_libraries(mprpc_dependencies INTERFACE mprpc_proto_dependencies)
endif()
