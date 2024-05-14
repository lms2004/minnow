#include "byte_stream.hh"
#include <iostream>
using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ), buffer_() {}

bool Writer::is_closed() const
{
  return close_;
}

void Writer::push( string data )
{
  if ( has_error() ) {
    close();
    return;
  }
  uint64_t available_space = available_capacity();
  uint64_t data_size = data.size();
  uint64_t push_size = std::min( available_space, data_size );

  bytes_pushed_ += push_size;
  buffer_ += std::move( data.substr( 0, push_size ) );
  return;
}

void Writer::close()
{
  close_ = true;
  return;
}

uint64_t Writer::available_capacity() const
{
  return capacity_ - buffer_.size();
}

uint64_t Writer::bytes_pushed() const
{
  return bytes_pushed_;
}

bool Reader::is_finished() const
{
  if ( close_ && !buffer_.size() ) {
    return true;
  } else {
    return false;
  }
}

uint64_t Reader::bytes_popped() const
{
  return bytes_popped_;
}

std::string_view Reader::peek() const
{
  if ( !buffer_.empty() ) {

    return std::string_view( &buffer_[0], buffer_.size() );
  } else {

    return std::string_view();
  }
}

void Reader::pop( uint64_t len )
{
  if ( has_error() || is_finished() ) {
    return;
  }
  uint64_t popped_len = min( len, bytes_buffered() );
  bytes_popped_ += popped_len;
  buffer_ = buffer_.substr( popped_len );
  return;
}

uint64_t Reader::bytes_buffered() const
{
  return buffer_.size();
}
