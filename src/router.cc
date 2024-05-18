#include "router.hh"

#include <iostream>
#include <limits>

using namespace std;

// route_prefix: 要匹配数据报目标地址的 "最多32位" 的 IPv4 地址前缀
// prefix_length: 该路由条目适用的前提是前缀长度是多少，即路由前缀的多少高位（最重要的）比特
//    需要与数据报目标地址的相应比特匹配
// next_hop: 下一跳的 IP 地址。如果网络直接连接到路由器，则此字段为空
//    （在这种情况下，下一跳地址应为数据报的最终目的地）
// interface_num: 发送数据报的接口索引
void Router::add_route( const uint32_t route_prefix,
                        const uint8_t prefix_length,
                        const optional<Address> next_hop,
                        const size_t interface_num )
{
  cerr << "DEBUG: adding route " << Address::from_ipv4_numeric( route_prefix ).ip() << "/"
       << static_cast<int>( prefix_length ) << " => " << ( next_hop.has_value() ? next_hop->ip() : "(direct)" )
       << " on interface " << interface_num << "\n";

  // 你的代码在这里
}

// 遍历所有接口，并将每个传入的数据报路由到其正确的输出接口
void Router::route()
{
  // 你的代码在这里
}
