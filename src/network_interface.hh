#pragma once
#include <chrono>

#include<unordered_map>
#include <queue>
#include <memory>

#include "address.hh"
#include "ethernet_frame.hh"
#include "ipv4_datagram.hh"

// 一个连接IP（互联网层或网络层）与Ethernet（网络访问层或链路层）的“网络接口”。

// 该模块是TCP/IP协议栈的最低层（将IP与较低层网络协议，例如Ethernet，连接起来）。
// 但是同一个模块也作为路由器的一部分被重复使用：
// 路由器通常具有许多网络接口，路由器的工作是在这些不同的接口之间路由Internet数据报。

// 网络接口将数据报（来自“客户”，例如TCP/IP协议栈或路由器）转换为Ethernet帧。
// 为了填写Ethernet目标地址，它会查找每个数据报的下一个IP跳的Ethernet地址，使用地址解析协议（ARP）进行请求。
// 在相反的方向上，网络接口接受Ethernet帧，检查它们是否是为其目的地的，如果是，则根据其类型处理有效载荷。
// 如果是IPv4数据报，网络接口将其传递给上层协议栈。如果是ARP请求或回复，网络接口会根据需要处理帧并学习或回复。
class NetworkInterface
{
public:
  // 一个物理输出端口的抽象，网络接口在其中发送Ethernet帧
  class OutputPort
  {
  public:
    virtual void transmit(const NetworkInterface& sender, const EthernetFrame& frame) = 0;
    virtual ~OutputPort() = default;
  };

  // 使用给定的Ethernet（网络访问层）和IP（互联网层）地址构造网络接口
  NetworkInterface(std::string_view name,
                   std::shared_ptr<OutputPort> port,
                   const EthernetAddress& ethernet_address,
                   const Address& ip_address);

  // 发送Internet数据报，封装在Ethernet帧中（如果已知Ethernet目标地址）。
  // 将使用ARP查找下一跳的Ethernet目标地址。发送通过在帧上调用“transmit”（成员变量）完成。
  void send_datagram(const InternetDatagram& dgram, const Address& next_hop);

  // 接收Ethernet帧并做出适当的响应。
  // 如果类型是IPv4，则将数据报推送到datagrams_in队列。
  // 如果类型是ARP请求，则从“发送者”字段中学习映射，并发送ARP回复。
  // 如果类型是ARP回复，则从“发送者”字段中学习映射。
  void recv_frame(const EthernetFrame& frame);

  // 当时间流逝时定期调用
  void tick(size_t ms_since_last_tick);

  // 访问器
  const std::string& name() const { return name_; }
  const OutputPort& output() const { return *port_; }
  OutputPort& output() { return *port_; }
  std::queue<InternetDatagram>& datagrams_received() { return datagrams_received_; }

private:
  // IPv4创建以太网帧
  bool create_IPv4frame(const InternetDatagram& dgram, uint32_t next_hop, EthernetFrame& frame);
 
  // ARP创建以太网帧
  bool create_ARPframe(const ARPMessage & arp_msg);

  // 发送ARP请求获取目标MAC地址
  void send_arp_request(uint32_t next_hop_ip);

  bool reply_arp_request(const ARPMessage & arp_msg);

  void prints();

  // 接口的人类可读名称
  std::string name_;

  // 物理输出端口（+一个使用它发送Ethernet帧的辅助函数“传输”）
  std::shared_ptr<OutputPort> port_;
  void transmit(const EthernetFrame& frame) const { port_->transmit(*this, frame); }

  // 接口的Ethernet（也称为硬件、网络访问层或链路层）地址
  EthernetAddress ethernet_address_;

  // 接口的IP（也称为互联网层或网络层）地址
  Address ip_address_;

  // 已接收的数据报
  std::queue<InternetDatagram> datagrams_received_ {};

  // ARP映射
  std::unordered_map<uint32_t,EthernetAddress> ipToEthernetMap;

  // ARP请求时间队列
  std::unordered_map<uint32_t,uint64_t> recent_arp_requests;

  // 等待发送的数据报
  std::unordered_map<uint32_t,InternetDatagram> waiting_datagrams_;
  
  // 当前时间
  uint64_t currentTime = 0;
};
