// Copyright (c) 2022 Wenhao Kong
// https://github.com/WenhaoKong2001
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef HAILMAKER_HTTPSERVER_NET_TCP_CONNECTION_MANAGER_H_
#define HAILMAKER_HTTPSERVER_NET_TCP_CONNECTION_MANAGER_H_

#include <memory>
#include <unordered_map>
#include <utility>
#include <mutex>
#include <thread>
#include "acceptor.h"
#include "event_loop.h"
#include "tcp_connection.h"

class TcpConnectionManager {
 public:
  TcpConnectionManager(int thread_num);
  ~TcpConnectionManager();

  // Start working!
  void Start(AddrIpv4 addr);

  inline void SetOnMessageCallback(OnMessageCallBack cb) {
    on_message_callback_ = std::move(cb);
  }

  inline void SetOnConnectionCallback(OnConnectionCallBack cb) {
    on_connection_callback_ = std::move(cb);
  }

  void DeleteFromMap(uint64_t id);
 private:
  using ConnectionMap = std::unordered_map<uint64_t, std::shared_ptr<TcpConnection>>;
  void NewConnection(Socket sock, AddrIpv4 addr);
  int GetNextLoopId() const;

  EventLoop *reactor_;
  Acceptor acceptor_;
  ConnectionMap conn_map_;
  uint64_t next_conn_id_;
  OnMessageCallBack on_message_callback_;
  OnConnectionCallBack on_connection_callback_;
  std::mutex mu_;
  int thread_num_;
  std::vector<EventLoop *> sub_reactors_;
  std::vector<std::thread> loop_threads_;
};

#endif //HAILMAKER_HTTPSERVER_NET_TCP_CONNECTION_MANAGER_H_
