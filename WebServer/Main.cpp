#include <ctime>
#include <sstream>
#include <getopt.h>
#include <string>
#include <thread>
#include "EventLoop.h"
#include "Server.h"
#include "base/log/Logging.h"

std::string getFormattedDate() ;

int main(int argc, char *argv[]) {
  int threadNum = std::thread::hardware_concurrency()/2-1;
  std::string currentDate = getFormattedDate();
  int port = 8086;
  std::string logPath = "Webserver_" + currentDate + ".log";

  // parse args
  int opt;
  const char *str = "t:l:p:";
  while ((opt = getopt(argc, argv, str)) != -1) {
    switch (opt) {
      case 't': {
        threadNum = atoi(optarg);
        break;
      }
      case 'l': {
        logPath = optarg;
        if (logPath.size() < 2 || optarg[0] != '/') {
          printf("logPath should start with \"/\"\n");
          abort();
        }
        break;
      }
      case 'p': {
        port = atoi(optarg);
        break;
      }
      default:
        break;
    }
  }
  Logger::setLogFileName(logPath);
// STL库在多线程上应用
#ifndef _PTHREADS
  LOG << "_PTHREADS is not defined !";
#endif
  EventLoop mainLoop;
  Server myHTTPServer(&mainLoop, threadNum, port);
  LOG << "Starting server with the following settings:\n"
          << "Number of threads: " << threadNum << "\n"
          << "Log file path: " << logPath << "\n"
          << "Server port: " << port;
  myHTTPServer.start();
  mainLoop.loop();
  return 0;
}

std::string getFormattedDate() {
    // 获取当前时间
    std::time_t now = std::time(nullptr);
    // 将 time_t 格式转换为 tm 结构
    std::tm* now_tm = std::localtime(&now);

    // 创建一个 stringstream 来格式化日期
    std::stringstream ss;
    // 使用 tm 结构输出年月日，这里使用 ISO 8601 日期格式: YYYY-MM-DD
    ss << (now_tm->tm_year + 1900) << '-'
       << (now_tm->tm_mon + 1) << '-'
       << now_tm->tm_mday;

    // 返回格式化后的日期字符串
    return ss.str();
}