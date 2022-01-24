// Copyright (c) 2022 Wenhao Kong
// https://github.com/WenhaoKong2001
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef HAILMAKER_HTTPSERVER_NET_CHANNEL_H_
#define HAILMAKER_HTTPSERVER_NET_CHANNEL_H_

#include <sys/epoll.h>
#include <functional>
#include <utility>

class EventLoop;

class Channel {
 public:
  enum class ChannelStatus { POLLING, DELETED };
  using CallBack = std::function<void()>;
  explicit Channel(EventLoop *loop, int fd);

  inline void SetReadCallBack(CallBack r_cb) {
    read_callback_ = std::move(r_cb);
  }

  inline void SetWriteCallBack(CallBack w_cb) {
    write_callback_ = std::move(w_cb);
  }

  inline void EnableReading() {
    events_ |= EPOLLIN;
    Update();
  }

  inline void EnableWriting() {
    events_ |= EPOLLOUT;
    Update();
  }

  inline void EnableReadingAndWriting() {
    events_ |= EPOLLIN;
    events_ |= EPOLLOUT;
    Update();
  }

  inline uint32_t GetEvent() const {
    return events_;
  }

  inline int GetFd() const {
    return fd_;
  }

  inline void SetRevent(uint32_t event) {
    revent_ = event;
  }

  inline ChannelStatus GetChannelStatus() {
    return status_;
  }

  inline void SetChannelStatus(ChannelStatus net_status) {
    status_ = net_status;
  }

  inline void RemoveFromPoller();

  void Update();

  void HandleEvent();
 private:
  int fd_;
  uint32_t events_;
  uint32_t revent_;
  EventLoop *loop_;
  bool handling_event_;
  ChannelStatus status_;
  CallBack read_callback_;
  CallBack write_callback_;
};

#endif //HAILMAKER_HTTPSERVER_NET_CHANNEL_H_
