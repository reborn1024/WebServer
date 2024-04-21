#pragma once
#include <map>
#include <mutex>
#include <unordered_map>
#include "TcpConnection.h"


class EventLoop;
class TimerNode;
class Channel;

enum ProcessState {
  STATE_PARSE_URI = 1,
  STATE_PARSE_HEADERS,
  STATE_RECV_BODY,
  STATE_ANALYSIS,
  STATE_FINISH
};

enum URIState {
  PARSE_URI_AGAIN = 1,
  PARSE_URI_ERROR,
  PARSE_URI_SUCCESS,
};

enum HeaderState {
  PARSE_HEADER_SUCCESS = 1,
  PARSE_HEADER_AGAIN,
  PARSE_HEADER_ERROR
};

enum AnalysisState { ANALYSIS_SUCCESS = 1, ANALYSIS_ERROR };

enum ParseState {
  H_START = 0,
  H_KEY,
  H_COLON,
  H_SPACES_AFTER_COLON,
  H_VALUE,
  H_CR,
  H_LF,
  H_END_CR,
  H_END_LF
};


enum HttpMethod { METHOD_POST = 1, METHOD_GET, METHOD_HEAD };

enum HttpVersion { HTTP_10 = 1, HTTP_11 };

class MimeType {
 private:
  static std::unordered_map<std::string, std::string> mime;
  static std::once_flag once_control;

  // 私有化构造函数，防止创建实例
  MimeType() {}
  MimeType(const MimeType &m) {}

  // 初始化MIME类型映射表的函数
  static void init() {
    mime[".html"] = "text/html";
    mime[".avi"] = "video/x-msvideo";
    mime[".bmp"] = "image/bmp";
    mime[".c"] = "text/plain";
    mime[".doc"] = "application/msword";
    mime[".gif"] = "image/gif";
    mime[".gz"] = "application/x-gzip";
    mime[".htm"] = "text/html";
    mime[".ico"] = "image/x-icon";
    mime[".jpg"] = "image/jpeg";
    mime[".png"] = "image/png";
    mime[".txt"] = "text/plain";
    mime[".mp3"] = "audio/mp3";
    mime["default"] = "text/html"; // 默认MIME类型
  }

 public:
  // 获取指定后缀的MIME类型，如果没有找到则返回默认MIME类型
  static std::string getMime(const std::string &suffix) {
    std::call_once(once_control, MimeType::init); // 确保初始化函数只被调用一次
    auto it = mime.find(suffix);
    if (it == mime.end())
      return mime["default"];
    else
      return it->second;
  }
};

class HttpData : public TcpConnection {
public:
  HttpData(EventLoop *loop, int connfd);
  ~HttpData() { close(fd_); }
  void reset();

private:
  std::string inBuffer_;
  std::string outBuffer_;
  bool error_;

  HttpMethod method_;
  HttpVersion HTTPVersion_;
  std::string fileName_;
  std::string path_;
  
  int nowReadPos_;// 当前读取位置
  ProcessState state_;// 处理请求的状态
  ParseState hState_;// 头部解析状态
  bool keepAlive_;// 是否保持连接活跃
  std::map<std::string, std::string> headers_;// 存储头部字段的映射表

private:
  void handleRead() override;// 处理读操作的函数
  void handleWrite() override; // 处理写操作的函数
  void handleConn() override;// 处理新的连接的函数
  void handleError(int fd, int err_num, std::string short_msg) override;// 处理错误的函数
  URIState parseURI();// 解析URI的状态机函数
  HeaderState parseHeaders(); // 解析头部的状态机函数
  AnalysisState analysisRequest();// 分析请求的函数
};