#include "tcp_sender.hh"
#include "tcp_config.hh"
#include<iostream>

using namespace std;

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  return unAckedSegmentsNums;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  uint64_t exponent = 0;
  uint64_t number = initial_RTO_ms_ / raw_RTO_ms;
  while (number > 1) {
      number /= 2;
      exponent++;
  }

  return exponent + is_RTO_double;
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  TCPSenderMessage message;
  message.FIN = false;
  message.RST = input_.has_error();
  message.SYN = false;
  message.payload = "";
  message.seqno = currentSeqNum_ ;
  return message;
}


void TCPSender::push( const TransmitFunction& transmit )
{
  
  uint64_t windowSize = window_size_==0?1:window_size_;

  // 若ByteStream更新新字节,构造发送信息
  uint64_t bytes_to_send = input_.reader().bytes_buffered();  // 总共需要发送的字节长度
  uint64_t payload_len = min({input_.reader().bytes_buffered()
                            , static_cast<uint64_t>(TCPConfig::MAX_PAYLOAD_SIZE)
                            , static_cast<uint64_t>(windowSize) - sequence_numbers_in_flight()});

  TCPSenderMessage message =make_empty_message();
  
  // 处理SYN头
  if(!handleInitialSYN(message)){
    return;
  }
  
  do{
    if(static_cast<uint64_t>(windowSize) - sequence_numbers_in_flight() == 0){
      return ;
    }

    if(message.RST){
        transmit(message);
        return;
    }

    // 处理payload
    handlePayload(message);

    // 处理FIN
    if(handleFIN(message)){
      return ;
    }
    
    // 处理序列号
    handleSqeno(message);
    
    unAckedSegments[push_checkout] = message;

    // 增加未确认的分段数量
    push_checkout += payload_len;

    // 更新分段信息
    bytes_to_send -= payload_len;

    // 更新
    is_RTO_double = 0;

    // 发送信息
    transmit(message);

  }while(bytes_to_send >0);
}


void TCPSender::receive(const TCPReceiverMessage& msg) {
    // 检查返回message是否有错误，如果有则返回
    if (check_for_errors(msg)) {
        return;
    }

    // 检查返回message是否为无效ACK，如果是则返回
    if (is_invalid_ack(msg.ackno)) {
        return;
    }

    // 检查返回message是否为重复ACK，如果是则返回
    if (is_duplicate_ack(msg)) {
        return;
    }

    // 更新ACK信息
    update_ack_info(msg);

    // 释放已经被ACK的数据段
    handle_ack();

    // 更新窗口大小
    update_window_size(msg.window_size);

    // 更新未确认的段
    update_unacked_segments(msg.ackno);

    // 重置RTO相关状态
    resetRTO();
}



void TCPSender::tick(uint64_t ms_since_last_tick, const TransmitFunction& transmit)
{
    since_last_send += ms_since_last_tick;

    // 不会因为连续的零窗口确认而使 RTO 退避（不增加 RTO）。
    // 这种行为是为了维持连接和测试窗口是否已重新打开，而不是因为网络拥堵。
    if(window_size_ != 0 ){
      handle_RTO();
    }

    // 检查是否达到初始 RTO
    if (since_last_send >= initial_RTO_ms_) {
      since_last_send = 0;

      is_RTO_double = true;     

      // 传输未确认段
      if (!unAckedSegments.empty()) {
          transmit(unAckedSegments.begin()->second);
      }
    }
}


// Push:

// 进行“窗口探测”
void TCPSender::handleWindowProbe(const TransmitFunction& transmit) {
    if (!window_size_) {
        TCPSenderMessage message = make_empty_message();

        // 处理payload
        handlePayload(message);

        // 处理FIN
        if(handleFIN(message)){
          return ;
        }
        
        // 处理序列号
        handleSqeno(message);
        transmit(message);
    }
    
}

// 处理SYN头
bool TCPSender::handleInitialSYN(TCPSenderMessage& message) {
  // 流中无字节，且未结束传输
  if (isSYNSent_ && !input_.reader().bytes_buffered() && !input_.writer().is_closed()) {
      return false;  
  }

  //window_size还未设置，SYN已经设置
  if(!is_SYN_ACK && isSYNSent_){
    return false;  
  }

  if (!isSYNSent_) {
    message.SYN = true;
    isSYNSent_ = true;
  }
  return true;
}

// 处理分段FIN
bool TCPSender::handleFIN(TCPSenderMessage& message){  
  if(isFINSent_){
    return true;
  }     

  if (input_.writer().is_closed() &&
      !input_.reader().bytes_buffered() &&
      (window_size_ == 0 ? 1 : window_size_) - message.sequence_length() > 0){
      message.FIN = true;
      isFINSent_ = true;
  }

  return false; 
}

// 处理分段payload
void TCPSender::handlePayload(TCPSenderMessage& message){
  uint64_t payload_len = min({input_.reader().bytes_buffered()
                            , static_cast<uint64_t>(TCPConfig::MAX_PAYLOAD_SIZE)
                            , static_cast<uint64_t>(window_size_ == 0?1:window_size_) - sequence_numbers_in_flight()});

  // 处理分段的payload
  message.payload = std::string(input_.reader().peek().substr(0, payload_len));
  input_.reader().pop(payload_len);
}

// 处理分段序列号
void TCPSender::handleSqeno(TCPSenderMessage& message){    
    // 修改当前分段序列号
    message.seqno = currentSeqNum_;

    // 分段的序列号
    currentSeqNum_ = currentSeqNum_ + message.sequence_length();

    // 待确认的序列号数量
    unAckedSegmentsNums += message.sequence_length();
}

// receive

// 重新设置RTO
void TCPSender::resetRTO() {
    is_RTO_double = false;
    initial_RTO_ms_ = raw_RTO_ms;
    since_last_send = 0;
}

// 处理已经ack数据分段
void TCPSender::handle_ack() {
    auto it = unAckedSegments.begin();
    while (it != unAckedSegments.end()) {
        uint64_t seq_no = it->first;
        uint64_t end_seq_no = seq_no + it->second.sequence_length();
        if (end_seq_no < checkout) {  
            it = unAckedSegments.erase(it);
        } else {
            ++it;
        }
    }
}

// 检查返回message是否有错误
bool TCPSender::check_for_errors(const TCPReceiverMessage& msg) {
    // 如果ackno没有值并且窗口大小为0，设置输入流错误
    if (!msg.ackno.has_value()) {
        if (!msg.window_size) {
            input_.set_error();
        }
        return true;
    }

    // 如果接收到的消息有RST标志，设置输入流错误
    if (msg.RST) {
        input_.set_error();
        return true;
    }

    return false;
}

// 检查返回message是否为无效ACK
bool TCPSender::is_invalid_ack(const std::optional<Wrap32>& ackno) const {
    // 如果ackno大于当前序列号，则为无效ACK
    return ackno > currentSeqNum_;
}

// 检查返回message是否为重复ACK
bool TCPSender::is_duplicate_ack(const TCPReceiverMessage& msg) const {
    // 如果last_Ack_Seq有值且大于等于当前ackno并且窗口大小相同，则为重复ACK
    return last_Ack_Seq.has_value() &&
           last_Ack_Seq >= msg.ackno &&
           window_size_ == msg.window_size;
}

// 更新ACK信息
void TCPSender::update_ack_info(const TCPReceiverMessage& msg) {
    // 设置SYN确认标志
    is_SYN_ACK = true;
    // 更新最后的ACK序列号
    last_Ack_Seq = msg.ackno.value();
    // 解包ACK序列号并更新当前已确认的绝对序列号
    checkout = msg.ackno.value().unwrap(isn_, checkout);
}

// 更新窗口大小
void TCPSender::update_window_size(uint16_t new_window_size) {
    window_size_ = new_window_size;
}

// 更新未确认的段
void TCPSender::update_unacked_segments(const std::optional<Wrap32>& ackno) {
    if (ackno.has_value()) {
        // 计算并更新未确认的字节数
        unAckedSegmentsNums = currentSeqNum_.distance(ackno.value());
    }
}



// tick

//  调整重传超时时间
void TCPSender::handle_RTO(){
    if(is_RTO_double){
        initial_RTO_ms_ *=2;
        is_RTO_double = false;
    }
}


void print(TCPSenderMessage message){
    std::cout << "Current Sequence Number: " << message.seqno.getuint32_t() << std::endl;
    std::cout << "SYN: " << (message.SYN ? "true" : "false") << std::endl;
    std::cout << "payload: " << message.payload << std::endl;
    std::cout << "FIN: " << (message.FIN ? "true" : "false") << std::endl;
    std::cout << "RST: " << (message.RST ? "true" : "false") << std::endl;
    std::cout << "sequence_length: " << message.sequence_length() << std::endl;

}