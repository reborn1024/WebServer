set(SRCS
    HttpData.cpp
    Main.cpp
    Server.cpp
)
add_executable(WebServer ${SRCS})

# 链接库，注意顺序: 先依赖，后依赖者
target_link_libraries(WebServer libserver_net libserver_base)


add_subdirectory(base)
add_subdirectory(tests)
add_subdirectory(net)