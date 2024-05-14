#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"

#include <cstdint>
#include <functional>
#include <list>
#include <memory>
#include <optional>
#include <queue>
#include <map>

class TCPSender
{
public:
  /* 用给定的默认重传超时时间和可能的初始序列号构造TCP发送者 */
  TCPSender( ByteStream&& input, Wrap32 isn, uint64_t initial_RTO_ms )
    : input_( std::move( input ) ), isn_( isn ), initial_RTO_ms_( initial_RTO_ms ),raw_RTO_ms(initial_RTO_ms)
    ,currentSeqNum_(isn),last_Ack_Seq(isn)
    ,window_size_(2)
    ,unAckedSegments()
  {}


  /* 生成一个空的TCPSenderMessage */
  TCPSenderMessage make_empty_message() const;

  /* 接收并处理来自对端接收者的TCPReceiverMessage */
  void receive( const TCPReceiverMessage& msg );

  /* 定义`transmit`函数的类型，该函数用于push和tick方法发送消息 */
  using TransmitFunction = std::function<void( const TCPSenderMessage& )>;

  /* 从输出流中推送字节 */
  void push( const TransmitFunction& transmit );

  /* 自上次调用tick()方法以来，时间已经过去了指定的毫秒数 */
  void tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit );

  // 访问器
  uint64_t sequence_numbers_in_flight() const;  // 当前有多少序列号未确认？
  uint64_t consecutive_retransmissions() const; // 发生了多少次连续的重传？
  Writer& writer() { return input_.writer(); }
  const Writer& writer() const { return input_.writer(); }


  // 只读访问输入流的读取器（外部不能读取）
  const Reader& reader() const { return input_.reader(); }

private:
// push:

  // “窗口探测”
  void handleWindowProbe(const TransmitFunction& transmit);

  // 处理SYN头
  bool handleInitialSYN(TCPSenderMessage& message);

  // 处理分段payload
  void handlePayload(TCPSenderMessage& message);

  // 处理分段序列号
  void handleSqeno(TCPSenderMessage& message); 
  
  // 处理分段FIN
  bool handleFIN(TCPSenderMessage& message);

  // 重新设置RTO
  void resetRTO();
// Receive:

  // 处理返回的ACK
  void processACK(const TCPReceiverMessage& msg);

  // 处理已经ack数据分段
  void handle_ack();

// Tick:

  //  TCP 使用指数退避策略来调整重传超时时间
  void handle_RTO();


  // 构造函数中初始化的变量
  ByteStream input_; // 输入流
  Wrap32 isn_;       // 初始序列号
  uint64_t initial_RTO_ms_;   // 重传超时时间（毫秒）
  uint64_t raw_RTO_ms;        // 初始重传超时时间（毫秒


  Wrap32 currentSeqNum_;                    // 当前发送数据分段的序列号  
  std::optional<Wrap32> last_Ack_Seq;       // 上一个发送数据分段的序列号  

  uint16_t window_size_;                    // 当前接收方的窗口大小
  
  // 记录未确认的分段
  std::map<uint64_t,TCPSenderMessage> unAckedSegments;
  bool is_SYN_ACK = false;            // 记录窗口是否確定

  uint64_t unAckedSegmentsNums = 0;   // 当前待确认的字节数
  uint64_t checkout = 0;              // 当前已经ack的绝对序列号
  uint64_t push_checkout = 0;         // 当前已经push的绝对序列号

  uint64_t since_last_send = 0;       // 记录上次send的时间
  bool is_RTO_double = false;         // 记录非零窗口是否需要退避RTO（RTO增加）

  bool isSYNSent_= false;   // 判断是否发送过SYN
  bool isFINSent_= false;   // 判断是否发送过FIN
};


void print(TCPSenderMessage message);