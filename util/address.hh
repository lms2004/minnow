#pragma once

#include <cstddef>
#include <cstdint>
#include <netdb.h>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <utility>

//! 对 [IPv4 地址](@ref man7::ip) 和 DNS 操作进行封装的类。
class Address
{
public:
  //! \brief 对 [sockaddr_storage](@ref man7::socket) 的封装。
  //! \details `sockaddr_storage` 可以容纳任何套接字地址（IPv4 或 IPv6）。
  class Raw
  {
  public:
    sockaddr_storage storage {}; //!< 包装的结构体本身。
    // NOLINTBEGIN (*-explicit-*)
    operator sockaddr*();
    operator const sockaddr*() const;
    // NOLINTEND (*-explicit-*)
  };

private:
  socklen_t _size; //!< 包装地址的大小。
  Raw _address {}; //!< 包含地址的 [sockaddr_storage](@ref man7::socket) 的封装。

  //! 根据节点（ip/host）、服务（service/port）和解析器提示构造函数。
  Address( const std::string& node, const std::string& service, const addrinfo& hints );

public:
  //! 通过解析主机名和服务名构造。
  Address( const std::string& hostname, const std::string& service );

  //! 根据点分十进制字符串（"18.243.0.1"）和数值端口构造。
  explicit Address( const std::string& ip, std::uint16_t port = 0 );

  //! 根据 [sockaddr *](@ref man7::socket) 构造。
  Address( const sockaddr* addr, std::size_t size );

  //! 相等比较。
  bool operator==( const Address& other ) const;
  bool operator!=( const Address& other ) const { return not operator==( other ); }

  //! \name 转换
  //!@{

  //! 点分十进制 IP 地址字符串（"18.243.0.1"）和数值端口。
  std::pair<std::string, uint16_t> ip_port() const;
  //! 点分十进制 IP 地址字符串（"18.243.0.1"）。
  std::string ip() const { return ip_port().first; }
  //! 数值端口（主机字节序）。
  uint16_t port() const { return ip_port().second; }
  //! 数值 IP 地址作为整数（即，以 [主机字节序](\ref man3::byteorder)）。
  uint32_t ipv4_numeric() const;
  //! 从 32 位原始数值 IP 地址创建 Address。
  static Address from_ipv4_numeric( uint32_t ip_address );
  //! 人类可读字符串，例如 "8.8.8.8:53"。
  std::string to_string() const;
  //!@}

  //! \name 低级操作
  //!@{

  //! 底层地址存储的大小。
  socklen_t size() const { return _size; }
  //! 底层套接字地址存储的常量指针。
  const sockaddr* raw() const { return static_cast<const sockaddr*>( _address ); }
  //! 安全地转换为底层 sockaddr 类型
  template<typename sockaddr_type>
  const sockaddr_type* as() const;

  //!@}
};
