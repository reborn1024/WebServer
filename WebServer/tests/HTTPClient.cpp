#include <iostream>
#include <curl/curl.h>
#include <chrono>
#include <thread>
#include <vector>
#include <cstdlib>

bool VERBOSE = 0;

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void PerformHttpGet(CURL *curl, const std::string& url, bool keepAlive) {
    if (curl) {
        CURLcode res;
        std::string readBuffer;

        // 设置 URL 和回调函数
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);//超时时间
        if(VERBOSE)
          curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);//详细输出

        // 根据 keepAlive 变量的值设置长连接或短连接
        if (!keepAlive) {
            curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1L); // 短连接
        }

        // 执行 HTTP GET 请求
        res = curl_easy_perform(curl);

        // 检查错误
        if(res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        } else {
            std::cout << "Received response:" << std::endl;
            std::cout << "------------------------------" << std::endl;
            std::cout << readBuffer << std::endl; // 打印完整的响应报文
            std::cout << "------------------------------" << std::endl;
        }
    } else {
        std::cerr << "CURL could not be initialized." << std::endl;
    }
}

int main(int argc, char *argv[]) {
  std::string url = "127.0.0.1:8086/hello";
  bool keepAlive = 1;
  std::vector<int> waitTimes{0};
  for (int i = 1; i < argc; ++i) {
      std::string arg = argv[i];
      if (arg == "-u" && i + 1 < argc) { // 确认后面有另一个参数
          url = argv[++i]; // 在'-'后直接读取下一个参数作为值
      } else if (arg == "-v") {
        VERBOSE = true;
      } else if (arg == "-k" && i + 1 < argc) {
          keepAlive = std::stoi(argv[++i]) > 0;
      } else if (arg == "-t") {
          // 清除默认的waitTimes值，并用后续提供的数值填充
          waitTimes.clear();
          // 收集所有-t 后面的时间值直到遇到下一个参数或结束
          while(i + 1 < argc && argv[i + 1][0] != '-') {
              waitTimes.push_back(std::stoi(argv[++i]));
          }
      } else {
          std::cerr << "Unsupported argument: " << arg << std::endl;
          return 1;
      }
  }

  // 输出解析后的参数
  std::cout << "URL: " << url << std::endl;
  std::cout << "Keep-Alive: " << (keepAlive ? "Enabled" : "Disabled") << std::endl;
  
  std::cout << "Wait Times: ";
  for (const auto& time : waitTimes) {
      std::cout << time << " ";
  }
  std::cout << std::endl;

  curl_global_init(CURL_GLOBAL_ALL);
  CURL *curl = curl_easy_init();

  if (curl) {
      std::cout << "Starting HTTP requests..." << std::endl;

      for (int waitTime : waitTimes) {
          std::cout << "Waiting for " << waitTime << " second(s) before sending the next request..." << std::endl;
          
          // 等待指定时间
          std::this_thread::sleep_for(std::chrono::seconds(waitTime));

          std::cout << "Sending GET request to " << url << (keepAlive ? " with" : " without") << " keep-alive." << std::endl;

          // 发送GET请求
          PerformHttpGet(curl, url, keepAlive);

          if (!keepAlive) {
              // 短连接情况下，清理并重新初始化curl对象
              std::cout << "No keep-alive, cleaning up curl object and reinitializing..." << std::endl;
              curl_easy_cleanup(curl);
              curl = curl_easy_init();
              if (!curl) {
                  std::cerr << "Failed to reinitialize curl after cleanup." << std::endl;
                  break;
              }
          }
      }

      // 清理curl资源
      if (curl) {
          std::cout << "Cleaning up curl resources..." << std::endl;
          curl_easy_cleanup(curl);
      }
  } else {
      std::cerr << "Curl failed to initialize!" << std::endl;
  }

  curl_global_cleanup();
  std::cout << "Finished all HTTP requests." << std::endl;
  return 0;
}
