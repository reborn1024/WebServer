#pragma once
#include <sys/epoll.h>
#include <memory>
#include <unordered_map>
#include <vector>
#include "Channel.h"
#include "Timer.h"


class Epoll {
 public:
  Epoll();
  ~Epoll();
  void epoll_add(SP_Channel request, int timeout);
  void epoll_mod(SP_Channel request, int timeout);
  void epoll_del(SP_Channel request);
  std::vector<std::shared_ptr<Channel>> poll();
  std::vector<std::shared_ptr<Channel>> getEventsRequest(int events_num);
  void add_timer(std::shared_ptr<Channel> request_data, int timeout);
  int getEpollFd() { return epollFd_; }
  void handleExpired();

 private:
  static const int MAXFDS = 100000;
  int epollFd_; // epoll 句柄
  std::vector<epoll_event> events_;// epoll_wait()返回的活动事件都放在这个数组⾥
  std::shared_ptr<Channel> fd2chan_[MAXFDS];
  std::shared_ptr<TcpConnection> fd2http_[MAXFDS];
  TimerManager timerManager_;
};