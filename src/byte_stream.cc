#include "byte_stream.hh"
#include <algorithm>

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ), buffer_()
{
  buffer_.reserve( capacity_ );
}

bool Writer::is_closed() const
{
  return close_;
}

void Writer::push( std::string data )
{
  if ( has_error() ) {
    close();
    return;
  }

  uint64_t push_size = std::min( available_capacity(), data.size() );

  buffer_.insert( buffer_.end(), data.begin(), data.begin() + push_size );
  bytes_pushed_ += push_size;
}

void Writer::close()
{
  close_ = true;
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
  return close_ && buffer_.empty();
}

uint64_t Reader::bytes_popped() const
{
  return bytes_popped_;
}

std::string_view Reader::peek() const
{
  if ( !buffer_.empty() ) {
    return std::string_view( buffer_.data(), buffer_.size() );
  } else {
    return std::string_view();
  }
}

void Reader::pop( uint64_t len )
{
  if ( has_error() || is_finished() ) {
    return;
  }

  uint64_t pop_len = std::min( len, bytes_buffered() );
  buffer_.erase( buffer_.begin(), buffer_.begin() + pop_len );
  bytes_popped_ += pop_len;
}

uint64_t Reader::bytes_buffered() const
{
  return buffer_.size();
}
