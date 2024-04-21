#pragma once
#include "EventLoop.h"
#include <mutex>
#include <condition_variable>
#include "base/Thread.h"
#include "base/noncopyable.h"


class EventLoopThread : noncopyable {
 public:
  EventLoopThread();
  ~EventLoopThread();
  EventLoop* startLoop();

 private:
  void threadFunc();
  EventLoop* loop_;
  bool exiting_;
  Thread thread_;
  std::mutex mutex_;
  std::condition_variable cond_;
};