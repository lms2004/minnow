#pragma once

#include "parser.hh"

#include <cstddef>
#include <cstdint>
#include <string>

// IPv4互联网数据报头（注意：不支持IP选项）
struct IPv4Header
{
  static constexpr size_t LENGTH = 20;        // IPv4头部长度，不包括选项
  static constexpr uint8_t DEFAULT_TTL = 128; // 合理的默认TTL值
  static constexpr uint8_t PROTO_TCP = 6;     // TCP协议的协议号

  static constexpr uint64_t serialized_length() { return LENGTH; }

  /*
   *   0                   1                   2                   3
   *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   *  |Version|  IHL  |Type of Service|          Total Length         |
   *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   *  |         Identification        |Flags|      Fragment Offset    |
   *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   *  |  Time to Live |    Protocol   |         Header Checksum       |
   *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   *  |                       Source Address                          |
   *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   *  |                    Destination Address                        |
   *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   *  |                    Options                    |    Padding    |
   *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   */

  // IPv4头部字段
  uint8_t ver = 4;           // IP版本
  uint8_t hlen = LENGTH / 4; // 头部长度（32位的倍数）
  uint8_t tos = 0;           // 服务类型
  uint16_t len = 0;          // 数据包的总长度
  uint16_t id = 0;           // 标识号
  bool df = true;            // 不分片标志
  bool mf = false;           // 更多分片标志
  uint16_t offset = 0;       // 分片偏移字段
  uint8_t ttl = DEFAULT_TTL; // 存活时间字段
  uint8_t proto = PROTO_TCP; // 协议字段
  uint16_t cksum = 0;        // 校验和字段
  uint32_t src = 0;          // 源地址
  uint32_t dst = 0;          // 目标地址

  // 负载的长度
  uint16_t payload_length() const;

  // 伪头部对TCP校验和的贡献
  uint32_t pseudo_checksum() const;

  // 计算校验和的正确值
  void compute_checksum();

  // 返回以人类可读格式包含头部信息的字符串
  std::string to_string() const;

  void parse(Parser& parser);
  void serialize(Serializer& serializer) const;
};
