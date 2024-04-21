#pragma once
#include <sys/epoll.h>
#include <unistd.h>
#include <memory>


class EventLoop;
class TimerNode;
class Channel;

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
 public:
  TcpConnection(EventLoop *loop, int connfd);
  ~TcpConnection() { close(fd_); }
  
  void seperateTimer();                               // 分离计时器
  void linkTimer(std::shared_ptr<TimerNode> mtimer) { // 链接计时器，建立与TimerNode的联系
    timer_ = mtimer;
  }
  std::shared_ptr<Channel> getChannel() { return channel_; }
  EventLoop *getLoop() { return loop_; }
  void handleClose();
  void newEvent();
  enum ConnectionState { H_CONNECTED = 0, H_DISCONNECTING, H_DISCONNECTED };
  static const __uint32_t DEFAULT_EVENT = EPOLLIN | EPOLLET | EPOLLONESHOT;
  static const int DEFAULT_EXPIRED_TIME = 2000;              // ms
  static const int DEFAULT_KEEP_ALIVE_TIME = 5 * 60 * 1000;  // ms

protected:
  EventLoop *loop_;
  std::shared_ptr<Channel> channel_;
  int fd_;
  ConnectionState connectionState_;
  std::weak_ptr<TimerNode> timer_;// 计时器，使用弱引用以避免循环引用

private:

  virtual void handleRead() = 0;// 处理读操作的函数
  virtual void handleWrite() = 0; // 处理写操作的函数
  virtual void handleConn() = 0;// 处理新的连接的函数
  virtual void handleError(int fd, int err_num, std::string short_msg) = 0;// 处理错误的函数
};