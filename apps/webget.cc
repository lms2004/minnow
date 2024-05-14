#include "socket.hh"
#include "tcp_minnow_socket.hh"

#include <cstdlib>
#include <iostream>
#include <span>
#include <string>

using namespace std;

void get_URL( const std::string& host, const std::string& path )
{
  CS144TCPSocket client;
  Address server( host, "http" ); // Port should be 80 by default for HTTP

  client.connect( server );

  // Properly formatted HTTP GET request
  client.write( "GET " + path + " HTTP/1.1\r\n" );
  client.write( "Host: " + host + "\r\n" );
  client.write( "Connection: close\r\n\r\n" ); // Note the double \r\n

  std::string response;
  std::string buffer;
  while ( !client.eof() ) {
    client.read( buffer );
    response += buffer; // Append new data to the complete response
  }

  std::cout << response;
  client.close();
}

int main( int argc, char* argv[] )
{
  try {
    if ( argc <= 0 ) {
      abort(); // For sticklers: don't try to access argv[0] if argc <= 0.
    }

    auto args = span( argv, argc );

    // The program takes two command-line arguments: the hostname and "path" part of the URL.
    // Print the usage message unless there are these two arguments (plus the program name
    // itself, so arg count = 3 in total).
    if ( argc != 3 ) {
      cerr << "Usage: " << args.front() << " HOST PATH\n";
      cerr << "\tExample: " << args.front() << " stanford.edu /class/cs144\n";
      return EXIT_FAILURE;
    }

    // Get the command-line arguments.
    const string host { args[1] };
    const string path { args[2] };

    // Call the student-written function.
    get_URL( host, path );
  } catch ( const exception& e ) {
    cerr << e.what() << "\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
