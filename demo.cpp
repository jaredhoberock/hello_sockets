#include <cstring>
#include <limits.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <iostream>
#include <algorithm>
#include <system_error>
#include <cassert>


void write_int(const char* hostname, int port)
{
  // get the address of the server
  struct hostent* server = gethostbyname(hostname);
  if(server == nullptr)
  {
    throw std::system_error(errno, std::system_category(), "write_int(): Error after gethostbyname()");
  }

  // create a new socket
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if(socket_fd == -1)
  {
    throw std::system_error(errno, std::system_category(), "write_int(): Error after socket()");
  }

  sockaddr_in server_address{};

  server_address.sin_family = AF_INET;
  std::memcpy(&server_address.sin_addr.s_addr, server->h_addr, server->h_length);
  server_address.sin_port = port;

  // keep attempting a connection while the server refuses
  int attempt = 0;
  int connect_result = 0;
  while((connect_result = connect(socket_fd, reinterpret_cast<sockaddr*>(&server_address), sizeof(server_address))) == -1 && attempt < 1000)
  {
    if(errno != ECONNREFUSED)
    {
      throw std::system_error(errno, std::system_category(), "write_int(): Error after connect()");
    }

    ++attempt;
  }

  if(connect_result == -1)
  {
    throw std::system_error(errno, std::system_category(), "write_int(): Error after connect()");
  }

  std::cout << "Writer connected." << std::endl;

  // write a single int through the socket
  int write_me = 13;
  if(::write(socket_fd, &write_me, sizeof(write_me)) == -1)
  {
    throw std::system_error(errno, std::system_category(), "write_int(): Error after write()");
  }

  // close the socket
  if(close(socket_fd) == -1)
  {
    throw std::system_error(errno, std::system_category(), "write_int(): Error after close()");
  }
}

int read_int(int port)
{
  // create a new socket
  int listening_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if(listening_socket_fd == -1)
  {
    throw std::system_error(errno, std::system_category(), "read_int(): Error after socket()");
  }

  sockaddr_in server_address{};

  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  server_address.sin_port = port;

  // bind the socket to our selected port
  if(bind(listening_socket_fd, reinterpret_cast<const sockaddr*>(&server_address), sizeof(server_address)) == -1)
  {
    throw std::system_error(errno, std::system_category(), "read_int(): Error after bind()");
  }

  // make this socket a listening socket, listen for a single connection
  if(listen(listening_socket_fd, 1) == -1)
  {
    throw std::system_error(errno, std::system_category(), "read_int(): Error after listen()");
  }

  std::cout << "Reader listening for a connection..." << std::endl;

  // block until we have a connection
  int read_socket_fd = accept(listening_socket_fd, nullptr, nullptr);
  if(read_socket_fd == -1)
  {
    throw std::system_error(errno, std::system_category(), "read_int(): Error after accept()");
  }

  std::cout << "Reader connected." << std::endl;

  // close the listening socket
  if(close(listening_socket_fd) == -1)
  {
    throw std::system_error(errno, std::system_category(), "read_int(): Error after close()");
  }

  // read a single int from the read socket
  int result = 0;
  if(read(read_socket_fd, &result, sizeof(result)) == -1)
  {
    throw std::system_error(errno, std::system_category(), "read_int(): Error after read()");
  }

  // close the read socket
  if(close(read_socket_fd) == -1)
  {
    throw std::system_error(errno, std::system_category(), "read_int(): Error after close()");
  }

  return result;
}

int main()
{
  // get the name of this machine
  char hostname[HOST_NAME_MAX];
  if(gethostname(hostname, sizeof(hostname)) == -1)
  {
    throw std::system_error(errno, std::system_category(), "main(): Error after gethostname()");
  }

  // choose a port number arbitrarily
  int port = 71342;

  // create a separate process to act as the client
  if(fork() == 0)
  {
    // let the child process be the writer
    write_int(hostname, port);
    return 0;
  }
  else
  {
    // let the parent process be the reader
    int result = read_int(port);

    std::cout << "Parent process received " << result << std::endl;
    assert(result == 13);

    // wait on the writer process before continuing
    wait(nullptr);
  }
  
  std::cout << "OK" << std::endl;
}

