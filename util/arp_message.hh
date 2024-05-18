#pragma once

#include "ethernet_header.hh"
#include "ipv4_header.hh"
#include "parser.hh"

struct ARPMessage {
    static constexpr size_t LENGTH = 28; // ARP消息长度（字节）
    static constexpr uint16_t TYPE_ETHERNET = 1; // 以太网/Wi-Fi作为链路层协议的ARP类型
    static constexpr uint16_t OPCODE_REQUEST = 1; // 请求操作码
    static constexpr uint16_t OPCODE_REPLY = 2; // 回复操作码

    uint16_t hardware_type = TYPE_ETHERNET; // 链路层协议类型（一般为以太网/Wi-Fi）
    uint16_t protocol_type = EthernetHeader::TYPE_IPv4; // 网络层协议类型（一般为IPv4）
    uint8_t hardware_address_size = sizeof(EthernetHeader::src); // 硬件地址大小
    uint8_t protocol_address_size = sizeof(IPv4Header::src); // 协议地址大小
    uint16_t opcode {}; // 请求或回复操作码

    EthernetAddress sender_ethernet_address {}; // 发送方MAC地址
    uint32_t sender_ip_address {}; // 发送方IP地址

    EthernetAddress target_ethernet_address {}; // 目标MAC地址
    uint32_t target_ip_address {}; // 目标IP地址

    std::string to_string() const; // 返回ARP消息的人类可读格式
    bool supported() const; // 判断ARP消息是否被解析器支持
    void parse(Parser& parser); // 解析ARP消息
    void serialize(Serializer& serializer) const; // 序列化ARP消息
};

