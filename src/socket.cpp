#include "socket.h"

#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netdb.h>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

using namespace std;

Socket::Socket() {
  m_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (m_fd == -1)
    throw runtime_error("Couldn't create socket\n");

  const int reuse_address = 1;
  setsockopt(get_fd(), SOL_SOCKET, SO_REUSEADDR, &reuse_address,
             sizeof(reuse_address));
}

Socket::Socket(int fd) : m_fd(fd) {}

Socket::~Socket() { close(get_fd()); }

int Socket::get_fd() const { return m_fd; }

int Socket::bind(unsigned short port) const {
  sockaddr_in server = {0};
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(port);

  return ::bind(get_fd(), reinterpret_cast<sockaddr *>(&server),
                sizeof(server));
}

int Socket::listen() const { return ::listen(get_fd(), 10); }

std::unique_ptr<Socket> Socket::accept() const {
  socklen_t s = sizeof(get_fd());
  struct sockaddr_in client;
  const int accepted_fd =
      ::accept(get_fd(), reinterpret_cast<struct sockaddr *>(&client), &s);
  if (accepted_fd < 0) {
    throw runtime_error("Accept has failed");
  }
  return make_unique<Socket>(accepted_fd);
}

int Socket::connect(const std::string &host, unsigned short port) const {
  struct hostent *he(gethostbyname(host.c_str()));
  if (!he) {
    throw runtime_error("gethobyname failed for " + host);
  }

  struct sockaddr_in server = {0};
  memcpy(&server.sin_addr, he->h_addr_list[0], he->h_length);
  server.sin_family = AF_INET;
  server.sin_port = htons(port);

  return ::connect(get_fd(), (struct sockaddr *)&server, sizeof(server));
}

string Socket::recv() const {
  vector<char> buff(1024);
  ssize_t num_bytes = ::recv(get_fd(), &buff[0], buff.size(), 0);
  if (num_bytes < 0) {
    throw runtime_error("Failed to read from socket");
  }

  return std::string(&buff[0], num_bytes);
}

Receiver::Receiver(unsigned short port) : m_socket(make_unique<Socket>()) {
  if (m_socket->bind(port) < 0) {
    throw runtime_error("Bind failed\n");
  }
  if (m_socket->listen() < 0) {
    throw runtime_error("Listen failed");
  }
}

string Receiver::recv() {
  std::unique_ptr<Socket> accepted_socket(m_socket->accept());
  return accepted_socket->recv();
}

Sender::Sender(const string &host, unsigned short port)
    : m_host(host), m_port(port) {}

bool Sender::connect() {
  m_socket = make_unique<Socket>();
  return m_socket->connect(m_host, m_port) >= 0;
}

bool Sender::send(const string &msg) {
  if (!connect())
    return false;
  return ::send(m_socket->get_fd(), msg.c_str(), msg.size(), 0) >= 0;
}
