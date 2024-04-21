#pragma once
#include <sys/epoll.h>
#include <sys/epoll.h>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include "timer/Timer.h"

class EventLoop;
class TcpConnection;

class Channel {
 private:
  typedef std::function<void()> CallBack;
  EventLoop *loop_;
  int fd_;
  __uint32_t events_;// Channel正在监听的事件（或者说感兴趣的时间）
  __uint32_t revents_;// 返回的就绪事件
  __uint32_t lastEvents_;// 上⼀此事件（主要⽤于记录如果本次事件和上次事件⼀样 就没必要调⽤ epoll_mod

  // 方便找到上层持有该Channel的对象
  std::weak_ptr<TcpConnection> holder_;

 private:
  int parse_URI();
  int parse_Headers();
  int analysisRequest();

  CallBack readHandler_;
  CallBack writeHandler_;
  CallBack errorHandler_;
  CallBack connHandler_;

 public:
  Channel(EventLoop *loop);
  Channel(EventLoop *loop, int fd);
  ~Channel();
  int getFd();
  void setFd(int fd);

  // 设置和返回持有该Channel的对象
  void setHolder(std::shared_ptr<TcpConnection> holder) { holder_ = holder; }
  std::shared_ptr<TcpConnection> getHolder() {
    std::shared_ptr<TcpConnection> ret(holder_.lock());
    return ret;
  }

  // 设置回调
  void setReadHandler(CallBack &&readHandler) { readHandler_ = readHandler; }
  void setWriteHandler(CallBack &&writeHandler) { writeHandler_ = writeHandler; }
  void setErrorHandler(CallBack &&errorHandler) { errorHandler_ = errorHandler; }
  void setConnHandler(CallBack &&connHandler) { connHandler_ = connHandler; }

  void handleEvents() {
    events_ = 0;
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
      events_ = 0;
      return;
    }
    if (revents_ & EPOLLERR) {
      if (errorHandler_) errorHandler_();
      events_ = 0;
      return;
    }
    if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
      handleRead();
    }
    if (revents_ & EPOLLOUT) {
      handleWrite();
    }
    handleConn();
  }
  void handleRead();
  void handleWrite();
  void handleError(int fd, int err_num, std::string short_msg);
  void handleConn();

  void setRevents(__uint32_t ev) { revents_ = ev; }

  void setEvents(__uint32_t ev) { events_ = ev; }
  __uint32_t &getEvents() { return events_; }

  bool EqualAndUpdateLastEvents() {
    bool ret = (lastEvents_ == events_);
    lastEvents_ = events_;
    return ret;
  }

  __uint32_t getLastEvents() { return lastEvents_; }
};

typedef std::shared_ptr<Channel> SP_Channel;