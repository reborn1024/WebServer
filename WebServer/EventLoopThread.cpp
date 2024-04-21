#include "EventLoopThread.h"
#include <functional>

EventLoopThread::EventLoopThread()
    : loop_(NULL),
      exiting_(false),
      thread_(bind(&EventLoopThread::threadFunc, this), "EventLoopThread") {}

EventLoopThread::~EventLoopThread() {
  exiting_ = true;
  if (loop_ != NULL) {
    loop_->quit();
    thread_.join();
  }
}

EventLoop* EventLoopThread::startLoop() {
  assert(!thread_.started());
  thread_.start();
  {
    std::unique_lock<std::mutex> lock(mutex_);
    // 一直等到threadFun在Thread里真正跑起来
    while (loop_ == NULL) cond_.wait(lock);
  }
  return loop_;
}

void EventLoopThread::threadFunc() {
  EventLoop loop;

  {
    std::lock_guard<std::mutex> lock(mutex_);
    loop_ = &loop;
    cond_.notify_one();
  }

  loop.loop();
  // assert(exiting_);
  loop_ = NULL;
}