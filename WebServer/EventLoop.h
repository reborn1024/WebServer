#pragma once
#include <functional>
#include <memory>
#include <vector>
#include "Channel.h"
#include "Epoll.h"
#include "Util.h"
#include "base/CurrentThread.h"
#include "base/Logging.h"
#include "base/Thread.h"


#include <iostream>
using namespace std;

class EventLoop {
 public:
  typedef std::function<void()> Functor;
  EventLoop();// 初始化poller, event_fd，给 event_fd 注册到 epoll 中并注册其事件处理回调
  ~EventLoop();
  void loop();// 开始事件循环 调⽤该函数的线程必须是该 EventLoop 所在线程，也就是 Loop 函数不能跨线程调⽤
  void quit();// 停⽌ Loop
  void runInLoop(Functor&& cb);// 如果当前线程就是创建此EventLoop的线程 就调⽤callback(关闭连接 EpollDel) 否则就放⼊等待执⾏函数区
  void queueInLoop(Functor&& cb);// 把此函数放⼊等待执⾏函数区 如果当前是跨线程 或者正在调⽤等待的函数则唤醒
  bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }// 检查是否在创建loop的线程内。
  void assertInLoopThread() { assert(isInLoopThread()); }// 断言是否在创建loop的线程内
  void shutdown(shared_ptr<Channel> channel) { shutDownWR(channel->getFd()); }// 只关闭连接(此时还可以把缓冲区数据写完再关闭)
  void removeFromPoller(shared_ptr<Channel> channel) {// 从epoll内核事件表中删除fd及其绑定的事件
    // shutDownWR(channel->getFd());
    poller_->epoll_del(channel);
  }
  void updatePoller(shared_ptr<Channel> channel, int timeout = 0) {// 在epoll内核事件表修改fd所绑定的事件
    poller_->epoll_mod(channel, timeout);
  }
  void addToPoller(shared_ptr<Channel> channel, int timeout = 0) {// 把fd和绑定的事件注册到epoll内核事件表
    poller_->epoll_add(channel, timeout);
  }

 private:
  // 声明顺序 wakeupFd_ > pwakeupChannel_
  bool looping_;                          // 是否处于事件循环中
  shared_ptr<Epoll> poller_;              // epoll实例
  int wakeupFd_;                          // 用于唤醒事件循环的文件描述符
  shared_ptr<Channel> pwakeupChannel_;    // 管理wakeupFd_的channel
  bool quit_;                             // 是否退出事件循环的标志
  bool eventHandling_;                    // 是否正在处理事件
  mutable MutexLock mutex_;               // 互斥锁
  std::vector<Functor> pendingFunctors_;  // 待处理的回调函数列表
  bool callingPendingFunctors_;           // 是否正在调用待处理的回调函数
  const pid_t threadId_;                  // 创建EventLoop的线程ID

  void wakeup();
  void handleRead();
  void doPendingFunctors();
  void handleConn();
};
