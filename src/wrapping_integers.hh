#pragma once

#include <cstdint>
#include <iostream>

/*
 * Wrap32 类型表示一个 32 位无符号整数，具有以下特性：
 *    - 以任意“零点”（初始值）开始，并且
 *    - 当它达到 2^32 - 1 时会回绕到零。
 */

class Wrap32
{
public:
  explicit Wrap32( uint32_t raw_value ) : raw_value_( raw_value ) {}

  /* 根据绝对序列号 n 和零点构造一个 Wrap32。*/
  static Wrap32 wrap( uint64_t n, Wrap32 zero_point );

  /*
   * unwrap 方法根据零点和一个“检查点”（即接近期望答案的另一个绝对序列号），返回一个绕过这个 Wrap32 的绝对序列号。
   *
   * 有许多可能的绝对序列号，它们都绕过相同的 Wrap32。unwrap 方法应返回最接近检查点的那个。
   */
  uint64_t unwrap( Wrap32 zero_point, uint64_t checkpoint ) const;
  uint64_t distance( Wrap32 other );
  Wrap32 operator+( uint32_t n ) const { return Wrap32 { raw_value_ + n }; }
  bool operator==( const Wrap32& other ) const { return raw_value_ == other.raw_value_; }

  bool operator>( const Wrap32& other ) const { return raw_value_ > other.raw_value_; }
  bool operator>=( const Wrap32& other ) const { return raw_value_ >= other.raw_value_; }

  uint32_t getuint32_t() const { return raw_value_; }

protected:
  uint32_t raw_value_ {};
};
