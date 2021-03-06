// Copyright (c) 2022 Wenhao Kong
// https://github.com/WenhaoKong2001
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <assert.h>
#include "epoller.h"
#include "channel.h"

Epoller::Epoller() : epoll_fd_(epoll_create(MAX_EVENTS)) {
  events_ = new epoll_event[MAX_EVENTS];
}

Epoller::~Epoller() {
  delete[] events_;
}

std::vector<Channel *> Epoller::Poll() {
  std::vector<Channel *> res;

  auto ret = epoll_wait(epoll_fd_, events_, MAX_EVENTS, -1);
  if (ret > 0) {
    LOG_INFO("polled %d events", ret);
    for (int i = 0; i < ret; i++) {
      auto channel = (Channel *) events_[i].data.ptr;
      channel->SetRevent(events_[i].events);
      res.push_back(channel);
    }
  } else {
    LOG_FATAL("Epoller::Poll()  epoll_wait fatal");
  }

  return res;
}

void Epoller::UpdateChannel(Channel *channel) {
  if (channel->GetChannelStatus() == Channel::ChannelStatus::DELETED) {
    struct epoll_event event{};
    event.data.ptr = channel;
    event.events = channel->GetEvent();
    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, channel->GetFd(), &event);
    channel->SetChannelStatus(Channel::ChannelStatus::POLLING);
  } else {
    struct epoll_event event{};
    event.data.ptr = channel;
    event.events = channel->GetEvent();
    epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, channel->GetFd(), &event);
  }
}

void Epoller::RemoveFromChannel(Channel *channel) {
  if (channel->GetChannelStatus() == Channel::ChannelStatus::POLLING) {
    epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, channel->GetFd(), nullptr);
    channel->SetChannelStatus(Channel::ChannelStatus::DELETED);
  }
}

