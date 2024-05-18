#pragma once

#include "ipv4_header.hh"
#include "parser.hh"

#include <memory>
#include <string>
#include <vector>

//! \brief [IPv4](\ref rfc::rfc791)互联网数据报
struct IPv4Datagram
{
  IPv4Header header {}; // IPv4头部
  std::vector<std::string> payload {}; // 数据负载

  // 解析IPv4数据报
  void parse(Parser& parser)
  {
    header.parse(parser); // 解析头部信息
    parser.all_remaining(payload); // 解析剩余数据作为负载
  }

  // 序列化IPv4数据报
  void serialize(Serializer& serializer) const
  {
    header.serialize(serializer); // 序列化头部信息
    for (const auto& x : payload) {
      serializer.buffer(x); // 序列化负载数据
    }
  }
};

// 互联网数据报别名
using InternetDatagram = IPv4Datagram;
