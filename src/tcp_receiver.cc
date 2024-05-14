#include "tcp_receiver.hh"
#include <iostream>
using namespace std;

/**
 * 目的：处理接收到的TCP消息，根据消息的标志位（如SYN, RST, FIN）进行相应的处理。
 * 功能：根据接收到的TCP段的状态标志，更新接收器的状态，并将有效负载数据插入到接收缓冲区中。
 *
 * @param message 包含TCP段信息
 * 包括序列号、
 * 同步标志(SYN)、
 * 数据负载、
 * 结束标志(FIN)
 * 以及重置标志(RST)。
 *
 */
void TCPReceiver::receive( TCPSenderMessage message )
{
  // 直接检查和处理 RST 标志
  if ( message.RST ) {
    reassembler_.reader().set_error();
    RST_ = reassembler_.reader().has_error();
    return;
  } else if ( RST_ ) {
    return;
  }

  // 处理 SYN 标志
  if ( message.SYN && !is_zero_point_set ) {
    zero_point = message.seqno;
    message.seqno = message.seqno + 1; // 如果有SYN，取下一位序列号
    is_zero_point_set = true;
  }

  // 如果 zero_point 尚未设置，则不继续执行
  if ( !is_zero_point_set ) {
    return;
  }

  // 获取当前数据首绝对序列号 ( >0 ) 已排除SYN
  uint64_t first_index = message.seqno.unwrap( zero_point, reassembler_.writer().bytes_pushed() );

  // 如果为0，说明当前数据报payload序列号在SYN的位置，无效
  if ( first_index == 0 ) {
    return;
  } else {
    first_index--;
  }

  bool FIN = message.FIN;

  // 插入到btyestream
  reassembler_.insert( first_index, message.payload, FIN );

  // 下一个相对序列号
  next_ackno
    = zero_point + is_zero_point_set + reassembler_.writer().bytes_pushed() + reassembler_.writer().is_closed();
}

/**
 * 目的：生成TCP接收器的状态消息，用于反馈给发送方。
 * 功能：构建并返回包含确认号、窗口大小和RST标志的TCP接收器消息。
 *
 * @return TCPReceiverMessage 结构体，包含确认号、窗口大小和RST状态。
 */
TCPReceiverMessage TCPReceiver::send() const
{
  TCPReceiverMessage ReceiverMessage;

  // 赋值 下一个相对序列号ackno
  if ( is_zero_point_set ) {
    ReceiverMessage.ackno = next_ackno;
  }

  // 赋值RST
  ReceiverMessage.RST = reassembler_.reader().has_error();

  // window_size 表示bytestream里的可存储的字节数
  if ( reassembler_.writer().available_capacity() > UINT16_MAX ) {
    ReceiverMessage.window_size = UINT16_MAX;
  } else {
    ReceiverMessage.window_size = reassembler_.writer().available_capacity();
  }

  return ReceiverMessage;
}
