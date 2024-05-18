#pragma once

#include "parser.hh"

#include <array>
#include <cstdint>
#include <string>

// 以太网地址的辅助类型（六个字节的数组）
using EthernetAddress = std::array<uint8_t, 6>;

// 以太网广播地址（ff:ff:ff:ff:ff:ff）
constexpr EthernetAddress ETHERNET_BROADCAST = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

// 以太网地址的可打印表示
std::string to_string( EthernetAddress address );

// 以太网帧头部
struct EthernetHeader
{
  static constexpr size_t LENGTH = 14;         //!< 以太网头部长度（字节）
  static constexpr uint16_t TYPE_IPv4 = 0x800; //!< [IPv4](\ref rfc::rfc791)的类型号
  static constexpr uint16_t TYPE_ARP = 0x806;  //!< [ARP](\ref rfc::rfc826)的类型号

  EthernetAddress dst; // 目标地址
  EthernetAddress src; // 源地址
  uint16_t type;       // 类型字段

  // 返回以人类可读格式包含头部信息的字符串
  std::string to_string() const;

  // 解析头部信息
  void parse( Parser& parser );

  // 序列化头部信息
  void serialize( Serializer& serializer ) const;
};
