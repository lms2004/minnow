#pragma once

#include <memory>
#include <optional>

#include "exception.hh"
#include "network_interface.hh"

// \brief 一个拥有多个网络接口的路由器，并在它们之间执行最长前缀匹配路由
class Router
{
public:
  // 向路由器添加一个接口
  // \param[in] interface 一个已构建的网络接口
  // \returns 接口被添加到路由器后的索引
  size_t add_interface( std::shared_ptr<NetworkInterface> interface )
  {
    _interfaces.push_back( notnull( "add_interface", std::move( interface ) ) );
    return _interfaces.size() - 1;
  }

  // 按索引访问接口
  std::shared_ptr<NetworkInterface> interface( const size_t N ) { return _interfaces.at( N ); }

  // 添加一条路由（转发规则）
  void add_route( uint32_t route_prefix,
                  uint8_t prefix_length,
                  std::optional<Address> next_hop,
                  size_t interface_num );

  // 在接口之间路由数据包
  void route();

private:
  // 路由器的网络接口集合
  std::vector<std::shared_ptr<NetworkInterface>> _interfaces {};
};
