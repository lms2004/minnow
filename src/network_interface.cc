#include <iostream>

#include "arp_message.hh"
#include "exception.hh"
#include "network_interface.hh"

using namespace std;

//! \param[in] ethernet_address 接口的Ethernet（ARP称为“硬件”）地址
//! \param[in] ip_address 接口的IP（ARP称为“协议”）地址
NetworkInterface::NetworkInterface(string_view name,
                                   shared_ptr<OutputPort> port,
                                   const EthernetAddress& ethernet_address,
                                   const Address& ip_address)
    : name_(name),
      port_(notnull("OutputPort", move(port))),
      ethernet_address_(ethernet_address),
      ip_address_(ip_address),
      ipToEthernetMap(),
      recent_arp_requests(),
      waiting_datagrams_()
{
    cerr << "DEBUG: 网络接口具有以太网地址 " << to_string(ethernet_address) << " 和 IP 地址 " << ip_address.ip() << "\n";
}


//! \param[in] dgram 要发送的IPv4数据报
//! \param[in] next_hop 要发送到的接口的IP地址（通常是路由器或默认网关，但也可以是另一台主机，
//  如果直接连接到与目标相同的网络）
//  注意：可以使用Address::ipv4_numeric()方法将Address类型转换为uint32_t（原始的32位IP地址）。
void NetworkInterface::send_datagram(const InternetDatagram& dgram, const Address& next_hop)
{   
    uint32_t next_hop_ip = next_hop.ipv4_numeric();

    EthernetFrame frame;    
    // 若可查询到映射
    if(create_IPv4frame(dgram,next_hop_ip,frame)) {
        transmit(frame);
        return ;
    }


    // 如果未知目标以太网地址，发送ARP请求
    if (((currentTime - recent_arp_requests[next_hop_ip]) >= 5*1000) || 
        (!recent_arp_requests[next_hop_ip] && !currentTime)) {
        send_arp_request(next_hop_ip);
        recent_arp_requests[next_hop_ip] = currentTime;
    }

    // 待排队发送的的数据报
    waiting_datagrams_[next_hop_ip] = dgram;
}

//! \param[in] frame 输入的Ethernet帧
void NetworkInterface::recv_frame(const EthernetFrame& frame)
{   
    // 检查帧是否是发给我们的 或者广播
    if(frame.header.dst != ethernet_address_ && frame.header.dst != ETHERNET_BROADCAST){
            return ;
    }

   // 为IPV4数据报
    if(frame.header.type == frame.header.TYPE_IPv4){
        // 处理IPv4帧
        InternetDatagram dgram;
        if (parse(dgram, frame.payload)) {
            // 用于上层通信的数据报
            datagrams_received_.push(dgram);
        }
    }            
    // 为ARP数据报
    if(frame.header.type == frame.header.TYPE_ARP){
        // 处理ARP帧
        ARPMessage arp_msg;
        if (parse(arp_msg, frame.payload)) {
            ipToEthernetMap[arp_msg.sender_ip_address] = arp_msg.sender_ethernet_address;

            recent_arp_requests[arp_msg.sender_ip_address] = currentTime;
            // 如果是ARP请求并且目标是我们的IP地址，发送ARP回复
            if (reply_arp_request(arp_msg)) {
                return ;
            }

            // 如果是ARP回复且目标是我们的IP地址，处理排队的数据报
            if (create_ARPframe(arp_msg)) {
                return ;
            }
        }
   }
}


//! \param[in] ms_since_last_tick 自上次调用此方法以来的毫秒数
void NetworkInterface::tick(const size_t ms_since_last_tick)
{   currentTime += ms_since_last_tick;
    for (auto it = recent_arp_requests.begin(); it != recent_arp_requests.end(); ) {
        // ARP映射记录时间超过30秒删除
        if (it->second + 30*1000 <= currentTime) {
            ipToEthernetMap.erase(it->first);
            it = recent_arp_requests.erase(it);
            currentTime = 0;
        } else {
            ++it;
        }
    }
}



// IPv4创建以太网帧
bool NetworkInterface::create_IPv4frame(const InternetDatagram& dgram, uint32_t next_hop, EthernetFrame& frame)
{
    auto it = ipToEthernetMap.find(next_hop);
    if (it != ipToEthernetMap.end()) {
        frame.header.type = EthernetHeader::TYPE_IPv4;
        frame.header.dst = it->second;
        frame.header.src = ethernet_address_;

        // 序列化IPv4
        frame.payload = serialize(dgram);
        return true;
    }
    return false; // 返回 false 表示没有找到对应的以太网地址
}

// 利用ARP创建以太网帧
bool NetworkInterface::create_ARPframe(const ARPMessage & arp_msg){
    if(arp_msg.opcode == ARPMessage::OPCODE_REPLY 
                && arp_msg.target_ip_address == ip_address_.ipv4_numeric()){

        EthernetFrame frame_;
        auto it = waiting_datagrams_.find(arp_msg.sender_ip_address);       
        if (it != waiting_datagrams_.end()) {
            frame_.header.type = EthernetHeader::TYPE_IPv4;
            frame_.header.src = ethernet_address_;
            frame_.header.dst = arp_msg.sender_ethernet_address;
            frame_.payload = serialize(it->second);
            transmit(frame_);   
            waiting_datagrams_.erase(it); 
        }
        return true;
    }else{
        return false;
    }
}

// 发送ARP请求获取目标MAC地址
void NetworkInterface::send_arp_request(uint32_t next_hop_ip) {
    // 构建ARP请求
    ARPMessage arp_request;
    arp_request.opcode = ARPMessage::OPCODE_REQUEST;            // 操作码：请求
    
    arp_request.sender_ethernet_address = ethernet_address_;    // 发送方MAC地址
    arp_request.sender_ip_address = ip_address_.ipv4_numeric(); // 发送方IP地址
    
    arp_request.target_ethernet_address = EthernetAddress{0, 0, 0, 0, 0, 0}; // 目标MAC地址：全零
    arp_request.target_ip_address = next_hop_ip; // 目标IP地址

    // ARP的数据帧
    EthernetFrame frame;
    frame.header.src = ethernet_address_;
    frame.header.dst = ETHERNET_BROADCAST;
    frame.header.type = EthernetHeader::TYPE_ARP;
    
    // 序列化ARP请求
    Serializer serializer;
    arp_request.serialize(serializer);
    frame.payload = serializer.output();

    transmit(frame);
}

// 回复ARP请求
bool NetworkInterface::reply_arp_request(const ARPMessage & arp_msg){
    if(arp_msg.opcode == ARPMessage::OPCODE_REQUEST 
        && arp_msg.target_ip_address == ip_address_.ipv4_numeric()){
        // 构建reply ARP
        ARPMessage arp_reply;
        arp_reply.opcode = ARPMessage::OPCODE_REPLY;
        arp_reply.sender_ethernet_address = ethernet_address_;
        arp_reply.sender_ip_address = ip_address_.ipv4_numeric();
        arp_reply.target_ethernet_address = arp_msg.sender_ethernet_address;
        arp_reply.target_ip_address = arp_msg.sender_ip_address;

        EthernetFrame reply_frame;
        reply_frame.header.type = EthernetHeader::TYPE_ARP;
        reply_frame.header.src = ethernet_address_;
        reply_frame.header.dst = arp_msg.sender_ethernet_address;
        reply_frame.payload = serialize(arp_reply);  
        transmit(reply_frame);        
        return true;   
    }else{
        return false;
    }
}


void NetworkInterface::prints(){
    cout<<"当前所有映射 "<<endl;
    for(auto it = ipToEthernetMap.begin();it != ipToEthernetMap.end();it++){
        cout<<"IP序列:  "<<it->first<<endl;
        cout<<"MAC地址: "<<to_string(it->second)<<endl;
        cout<<"对应时间"<<recent_arp_requests[it->first]<<endl;
        cout<<"当前时间"<<currentTime<<endl;
    }
}