#include "reassembler.hh"
using namespace std;
#include <iostream>
/**
 * 目的： 调整数据溢出情况，确保数据不会超出缓冲区的容量限制。
 * 功能:  裁剪输入数据以适应剩余空间。
 *
  writer缓冲区空间：
 * [0 , unpopped_index)                  已经popped字节流区间
 * [unpopped_index , unassemble_index)   已经pushed，但未popped字节流区间
 * [unassemble_index , capacity_index)   剩余的容量字节流区间
 *
 * @param first_index 数据的起始索引。
 * @param data 需要处理的数据字符串，处理后可能会被修改以适应可存储区大小。
 * @return 调整后的数据范围，表示有效数据的开始和结束索引。
 *
 */
pair<uint64_t, uint64_t> Reassembler::handle_data_overflow( uint64_t first_index, std::string& data )
{
  // 获取当前可存储区的状态
  uint64_t unpopped_index = output_.writer().getUnpoppedIndex(); // 可存储区中已pop出的最后位置
  uint64_t capacity_index = unpopped_index + output_.writer().getCapacity(); // 可存储区的总容量
  uint64_t available_capacity = output_.writer().available_capacity();       // 可存储区剩余可用容量

  // 计算数据的实际结束索引
  uint64_t end_index = first_index + data.size(); // 数据结束索引

  uint64_t end = end_index;
  uint64_t first = first_index;
  if ( data.empty() ) {
    return { 0, 0 };
  }

  // 若 数据非空 并 容量不足 数据超出容量索引 该数据为已经被push段 退出
  if ( !data.empty()
       && ( !available_capacity || first_index >= capacity_index || end_index <= unassemble_index ) ) {
    data.clear();    // 如果数据无法处理，则清空
    return { 0, 0 }; // 返回无效范围
  }

  // 处理不在可存储区内的数据后部
  if ( end > capacity_index ) {
    end = min( end_index, capacity_index );
    data = data.erase( end - first_index );
  }
  // 处理不在可存储区内的数据前部
  if ( end <= capacity_index ) {
    first = max( first_index, unassemble_index );
    data = data.erase( 0, first - first_index );
  }

  // 返回有效数据的新索引范围
  return { first, end };
}

/*
 * 功能： 用于合并当前数据与前一个buffer里的数据
 */
bool Reassembler::merge_prev( uint64_t& first, uint64_t& end, string& data )
{
  auto it = buffer_.lower_bound( first );
  if ( it != buffer_.begin() ) {
    --it;
    string p_data = it->second;
    uint64_t p_end = it->first + p_data.size();
    // 该数据已经被存储，返回1以退出insert
    if ( !p_data.empty() && p_end >= end ) {
      return 1;
    }
    // 该数据与前段数据无重叠，返回0以继续insert
    if ( !p_data.empty() && first >= p_end ) {
      return 0;
    }

    data.erase( 0, p_end - first );
    p_data += std::move( data );
    first = it->first;
    data = std::move( p_data );
  }
  return 0;
}

/*
 * 功能： 用于合并当前数据与前一个buffer里的数据
 */
bool Reassembler::merge_next( uint64_t& first, uint64_t& end, string& data )
{
  auto next = buffer_.upper_bound( first );
  if ( next != buffer_.end() ) { // 检查是否超出后一个
    uint64_t n_first = next->first;
    string n_data = next->second;

    if ( !n_data.empty() && end >= n_first ) {
      buffer_.erase( next ); // 使用迭代器进行删除操作
      n_data.erase( 0, end - n_first );
      data += n_data;
      end += n_data.size();
    }
  }
  return 1;
}
/*
 * 功能：  删除buffer重叠的数据
 */
bool Reassembler::delete_repeating( uint64_t& first, uint64_t& end )
{
  auto temp = buffer_.begin();
  while ( temp != buffer_.end() ) {
    uint64_t temp_end = temp->first + temp->second.size();
    if ( temp->first < first ) {
      ++temp;
      continue;
    }

    if ( first == temp->first && end <= temp_end ) {
      return 1;
    }

    if ( end < temp_end ) {
      return 0;
    }

    if ( first <= temp->first && end >= temp_end ) {
      temp = buffer_.erase( temp );
    } else {
      ++temp;
    }
  }
  return false;
}

/*
 * 功能：合并缓冲区中重复数据
 */
bool Reassembler::merge_buffer_repeating( uint64_t& first, uint64_t& end, string& data )
{
  if ( !buffer_.empty() ) {
    if ( merge_prev( first, end, data ) ) {
      return 1;
    }
    if ( delete_repeating( first, end ) ) {
      return 1;
    }
    merge_next( first, end, data );
  }
  if ( !data.empty() ) {
    buffer_[first] = std::move( data );
  }
  return 0;
}
/*
 * 功能：  推送buffer里连续数据
 */
void Reassembler::push_buffer_data()
{
  auto it_ = buffer_.begin();
  while ( !buffer_.empty() && it_->first == unassemble_index ) {
    output_.writer().push( it_->second );
    unassemble_index += it_->second.size();
    buffer_.erase( it_++ );
  }
}
void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  uint64_t unpopped_index = output_.writer().getUnpoppedIndex();
  uint64_t capacity_index = unpopped_index + output_.writer().getCapacity();

  auto [first, end] = handle_data_overflow( first_index, data );

  if ( merge_buffer_repeating( first, end, data ) ) {
    return;
  }

  push_buffer_data();
  if ( is_last_substring && end <= capacity_index - 1 ) {
    eof = true;
  }
  if ( eof && buffer_.empty() ) {
    output_.writer().close();
    return;
  }
}

uint64_t Reassembler::bytes_pending() const
{
  uint64_t bytes = 0;
  for ( const auto& stream : buffer_ ) {
    bytes += stream.second.size();
  }
  return bytes;
}