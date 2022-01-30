// Copyright (c) 2022 Wenhao Kong
// https://github.com/WenhaoKong2001
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <cstring>
#include <csignal>
#include "tcp_connection.h"
#include "tcp_connection_manager.h"

TcpConnection::TcpConnection(uint64_t id, Socket sock, AddrIpv4 addr, EventLoop *loop, TcpConnectionManager *manager)
    : id_(id),
      sock_(sock),
      peer_addr_(addr),
      loop_(loop),
      channel_(loop_, sock_.GetFd()),
      statue_(Connecting),
      manager_(manager) {
  channel_.SetReadCallBack(std::bind(&TcpConnection::HandleRead, this));
  channel_.SetWriteCallBack(std::bind(&TcpConnection::HandleWrite, this));
}

void TcpConnection::CloseConnection() {
  // TODO
  LOG_DEBUG("[ID:%d]TcpConnection::CloseConnection", id_);
  if (statue_ != TcpConnectionStatue::Closed && statue_ != TcpConnectionStatue::Closing) {
    if (output_buf_.Empty()) {
      LOG_DEBUG("[ID:%d]TcpConnection::CloseConnection  class all", id_);
      CloseAll();
    } else {
      statue_ = TcpConnectionStatue::Closing;
      LOG_DEBUG("[ID:%d]TcpConnection::CloseConnection  set statue_ to closing", id_);
    }
  }
}

void TcpConnection::HandleRead() {
  LOG_DEBUG("[ID:%d]TcpConnection::HandleRead()", id_);
  if (statue_ != Closed) {
    if (statue_ == Closing) {
      LOG_DEBUG("[ID:%d]TcpConnection::HandleRead()  statue == closing", id_);
      if (output_buf_.Empty()) {
        LOG_DEBUG("[ID:%d]TcpConnection::HandleRead()  close all", id_);
        CloseAll();
      }
    } else {
      auto n = input_buf_.ReadFd(sock_.GetFd());
      if (n > 0) {
        LOG_DEBUG("[ID:%d]TcpConnection::HandleRead()  read %d bytes", id_, n);
        on_message_callback_(shared_from_this(), &input_buf_);
      } else if (n == 0) {
        LOG_DEBUG("[ID:%d]TcpConnection::HandleRead()  client closed the connection", id_);
        CloseConnection();
      } else {
        LOG_ERROR("[ID:%d]TcpConnection::HandleRead()  encounter error.close the connection", id_);
        CloseConnection();
      }
    }
  }
}

void TcpConnection::Send(char *msg, size_t len) {
  // TODO
  if (statue_ == Closing || statue_ == Closed) {
    LOG_ERROR("[ID:%d]TcpConnection::Send  the connection is already closed.Cannot send more data", id_);
  } else {
    if (output_buf_.Empty() && !channel_.IsWriting()) {
      auto ret = sock_.Write(msg, len);
      if (ret > 0) {
        LOG_DEBUG("[ID:%d]TcpConnection::Send  send %d bytes directly", id_, ret);
        if (ret < len) {
          input_buf_.Append(msg + ret, len - ret);
          // Encountering EAGAIN or EWOULDBLOCK in write means sending buffer of tcp is full.
          // We just sign up a writing event of it,and waiting for the handleWrite function is called
          // when could write.
          if (errno == EAGAIN || errno == EWOULDBLOCK) {
            if (!channel_.IsWriting()) {
              channel_.EnableWriting();
            }
          }
          LOG_DEBUG("[ID:%d]TcpConnection::Send  send %d bytes and buffer is full,register writing event", id_, ret);
        }
      } else if (ret == 0) {
        // client close the connection
        LOG_DEBUG("[ID:%d]TcpConnection::Send  client close the connection", id_);
        CloseConnection();
      } else {
        // send buf of TCP is full registering POLLOUT event and waiting for it.
        LOG_DEBUG("[ID:%d]TcpConnection::Send  send buffer is full,register writing event", id_);
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
          if (!channel_.IsWriting()) {
            channel_.EnableWriting();
          }
        } else {
          // some error happened.Simply close the connection.
          LOG_ERROR("[ID:%d]TcpConnection::Send  some error happened.Simply close the connection.", id_);
          CloseConnection();
        }
      }
    } else {
      LOG_DEBUG("[ID:%d]TcpConnection::Send  append %n bytes to output_buf", id_, len);
      input_buf_.Append(msg, len);
    }
  }
}

void TcpConnection::HandleWrite() {
  LOG_DEBUG("[ID:%d]TcpConnection::HandleWrite", id_);
  if (statue_ != Closed) {
    if (channel_.IsWriting()) {
      auto readable = output_buf_.ReadableSize();
      auto n = ::write(sock_.GetFd(), output_buf_.Peek(), readable);
      if (n > 0) {
        LOG_DEBUG("[ID:%d]TcpConnection::HandleWrite  write %d bytes from buffer to client", id_, n);
        output_buf_.Retrieve(n);
        if (n == readable) {
          channel_.DisableWriting();
          LOG_DEBUG("[ID:%d]TcpConnection::HandleWrite  output buffer is empty disable writing event", id_);
          if (statue_ == Closing) {
            LOG_DEBUG("[ID:%d]TcpConnection::HandleWrite  close connection", id_);
            CloseAll();
          }
        }
      } else {
        LOG_ERROR("[ID:%d]TcpConnection::HandleWrite error close all", id_);
        CloseAll();
      }
    }
  }
}

void TcpConnection::ConnectionEstablished() {
  channel_.EnableReading();
  statue_ = Connected;
}

void TcpConnection::HandleCLose() {
  CloseAll();
}

void TcpConnection::CloseAll() {
  manager_->DeleteFromMap(id_);
  channel_.RemoveFromPoller();
  sock_.Close();
  statue_ = TcpConnectionStatue::Closed;
}






