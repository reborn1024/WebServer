# 指定net库的源文件
set(NET_SRCS
    Channel.cpp
    Epoll.cpp
    EventLoop.cpp
    EventLoopThread.cpp
    EventLoopThreadPool.cpp
    TcpConnection.cpp
    Util.cpp
    timer/Timer.cpp 
)
# 创建net库
add_library(libserver_net ${NET_SRCS})

# 链接base库
target_link_libraries(libserver_net libserver_base)

# 指定库输出名称
set_target_properties(libserver_net PROPERTIES OUTPUT_NAME "server_net")

target_include_directories(libserver_net PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})