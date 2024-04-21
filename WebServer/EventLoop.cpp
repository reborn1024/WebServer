#include "EventLoop.h"
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <iostream>
#include "Util.h"
#include "base/Logging.h"

using namespace std;

__thread EventLoop* t_loopInThisThread = 0; // __thread是GCC内置的线程局部存储设施，保证每个线程有自己独立实例

int createEventfd() {
  int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0) {
    LOG << "Failed in eventfd";
    abort();
  }
  return evtfd;
}

EventLoop::EventLoop()
    : looping_(false),
      poller_(new Epoll()),
      wakeupFd_(createEventfd()),
      pwakeupChannel_(new Channel(this, wakeupFd_)),
      quit_(false),
      eventHandling_(false),
      callingPendingFunctors_(false),
      threadId_(CurrentThread::tid()) {
  if (t_loopInThisThread) {
    // LOG << "Another EventLoop " << t_loopInThisThread << " exists in this
    // thread " << threadId_;
  } else {
    t_loopInThisThread = this;
  }
  // pwakeupChannel_->setEvents(EPOLLIN | EPOLLET | EPOLLONESHOT);
  pwakeupChannel_->setEvents(EPOLLIN | EPOLLET);
  pwakeupChannel_->setReadHandler([this](){this->handleRead();});// 绑定handleRead函数到读事件处理器
  pwakeupChannel_->setConnHandler([this](){this->handleConn();});// 绑定handleConn函数到连接事件处理器
  poller_->epoll_add(pwakeupChannel_, 0);
}

void EventLoop::handleConn() {
  // poller_->epoll_mod(wakeupFd_, pwakeupChannel_, (EPOLLIN | EPOLLET |
  // EPOLLONESHOT), 0);
  updatePoller(pwakeupChannel_, 0);
}

EventLoop::~EventLoop() {
  // wakeupChannel_->disableAll();
  // wakeupChannel_->remove();
  close(wakeupFd_);
  t_loopInThisThread = NULL;
}

// 唤醒EventLoop的函数，通过向eventfd写数据来触发
void EventLoop::wakeup() {
  uint64_t one = 1;
  ssize_t n = writen(wakeupFd_, (char*)(&one), sizeof one);
  if (n != sizeof one) {
    LOG << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
  }
}

// 读事件处理函数，从eventfd读取数据
void EventLoop::handleRead() {
  uint64_t one = 1;
  ssize_t n = readn(wakeupFd_, &one, sizeof one);
  if (n != sizeof one) {
    LOG << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
  }
  // pwakeupChannel_->setEvents(EPOLLIN | EPOLLET | EPOLLONESHOT);
  pwakeupChannel_->setEvents(EPOLLIN | EPOLLET);
}

void EventLoop::runInLoop(Functor&& cb) {
  if (isInLoopThread())
    cb();
  else
    queueInLoop(std::move(cb));
}

void EventLoop::queueInLoop(Functor&& cb) {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    pendingFunctors_.emplace_back(std::move(cb));
  }

  if (!isInLoopThread() || callingPendingFunctors_) wakeup();
}

void EventLoop::loop() {
  assert(!looping_); // 断言未处于循环中
  assert(isInLoopThread()); // 断言当前是在创建loop的线程中
  looping_ = true;
  quit_ = false;
  std::vector<SP_Channel> ret;  // 存放活跃事件的Channel列表
  while (!quit_) {
    ret.clear();
    ret = poller_->poll();                    // 调用Epoll的poll获取活跃事件列表
    eventHandling_ = true;                    // 标志正在处理事件
    for (auto& it : ret) it->handleEvents();  // 处理每个活跃事件
    eventHandling_ = false;                   // 标志事件处理完成
    doPendingFunctors();                      // 执行等待中的回调函数
    poller_->handleExpired();                 // 处理超时事件
  }
  looping_ = false;
}

void EventLoop::doPendingFunctors() {
  std::vector<Functor> functors;
  callingPendingFunctors_ = true;

  {
    std::lock_guard<std::mutex> lock(mutex_);
    functors.swap(pendingFunctors_);
  }

  for (size_t i = 0; i < functors.size(); ++i) functors[i]();
  callingPendingFunctors_ = false;
}

void EventLoop::quit() {
  quit_ = true;
  if (!isInLoopThread()) {
    wakeup();
  }
}