#pragma once

#include "ethernet_header.hh"
#include "parser.hh"

#include <vector>

// 以太网帧结构体
struct EthernetFrame
{
  EthernetHeader header {};            // 以太网帧头部
  std::vector<std::string> payload {}; // 数据负载

  // 解析以太网帧
  void parse( Parser& parser )
  {
    header.parse( parser );          // 解析帧头部
    parser.all_remaining( payload ); // 解析剩余数据作为负载
  }

  // 序列化以太网帧
  void serialize( Serializer& serializer ) const
  {
    header.serialize( serializer ); // 序列化帧头部
    serializer.buffer( payload );   // 序列化负载数据
  }
};
