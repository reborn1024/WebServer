#include "TcpConnection.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <iostream>
#include "Channel.h"
#include "EventLoop.h"
#include "Util.h"
#include "time.h"

using namespace std;

TcpConnection::TcpConnection(EventLoop *loop, int connfd)
    : loop_(loop),
      channel_(new Channel(loop, connfd)),
      fd_(connfd),
      connectionState_(H_CONNECTED) {}


void TcpConnection::seperateTimer() {
  // cout << "seperateTimer" << endl;
  if (timer_.lock()) {
    shared_ptr<TimerNode> my_timer(timer_.lock());
    my_timer->clearReq();
    timer_.reset();
  }
}

void TcpConnection::handleClose() {
  connectionState_ = H_DISCONNECTED;
  shared_ptr<TcpConnection> guard(shared_from_this());
  loop_->removeFromPoller(channel_);
}

void TcpConnection::newEvent() {
  channel_->setEvents(DEFAULT_EVENT);
  loop_->addToPoller(channel_, DEFAULT_EXPIRED_TIME);
}
