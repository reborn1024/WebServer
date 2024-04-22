add_executable(HTTPClient HTTPClient.cpp)

# 查找CURL库
find_package(CURL REQUIRED)

# 确保您的可执行文件链接到CURL库
target_link_libraries(HTTPClient ${CURL_LIBRARIES})