// Copyright (c) 2022 Wenhao Kong
// https://github.com/WenhaoKong2001
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "addr_ipv4.h"
#include <cstring>
#include <utils.h>
using namespace hailmaker;

AddrIpv4::AddrIpv4(uint16_t port) {
  bzero(&addr_, sizeof(addr_));
  addr_.sin_family = AF_INET;
  addr_.sin_port = sockets::HostToNet16(port);
  addr_.sin_addr.s_addr = INADDR_ANY;
}

AddrIpv4::AddrIpv4(const std::string &addr, uint16_t port) {
  bzero(&addr_, sizeof(addr_));
  addr_.sin_family = AF_INET;
  addr_.sin_port = sockets::HostToNet16(port);
  sockets::HostAddrToNet(addr.c_str(), &addr_);
}

void AddrIpv4::SetAddr(const sockaddr_in &net_addr) {

}

const struct sockaddr_in &AddrIpv4::GetAddr() {
  return addr_;
}

void AddrIpv4::GetStandardAddr() {

}
