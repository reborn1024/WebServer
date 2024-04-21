# 指定base库的源文件
set(LIB_SRC
    log/AsyncLogging.cpp
    log/FileUtil.cpp
    log/LogFile.cpp
    log/Logging.cpp
    log/LogStream.cpp
    CountDownLatch.cpp
    Thread.cpp
)
# 创建base库
add_library(libserver_base ${LIB_SRC})
target_link_libraries(libserver_base pthread rt)

#公共头文件
target_include_directories(libserver_base PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/log
)

# 设置base库输出名称
set_target_properties(libserver_base PROPERTIES OUTPUT_NAME "server_base")

add_subdirectory(tests)