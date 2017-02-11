#include "socket.h"
#include <fcntl.h>
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
  setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_address,
             sizeof(reuse_address));
}

Socket::Socket(int fd) : m_fd(fd) {}

Socket::~Socket() { close(m_fd); }

int Socket::get_fd() const { return m_fd; }

Receiver::Receiver(unsigned short port) : m_socket(make_unique<Socket>()) {
  sockaddr_in server = {0};
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(port);

  if (bind(m_socket->get_fd(), reinterpret_cast<sockaddr *>(&server),
           sizeof(server)) < 0) {
    throw runtime_error("Bind failed\n");
  }

  listen(m_socket->get_fd(), 10);
}

string Receiver::recv() {
  socklen_t s = sizeof(m_socket->get_fd());
  struct sockaddr_in client;
  const int accepted_fd = accept(
      m_socket->get_fd(), reinterpret_cast<struct sockaddr *>(&client), &s);
  if (accepted_fd < 0) {
    throw runtime_error("Accept has failed");
  }
  std::unique_ptr<Socket> accepted_socket(make_unique<Socket>(accepted_fd));

  vector<char> buff(1024);
  ssize_t num_bytes =
      ::recv(accepted_socket->get_fd(), &buff[0], buff.size(), 0);
  if (num_bytes < 0) {
    throw runtime_error("Failed to read from socket");
  }

  return std::string(&buff[0], num_bytes);
}

Sender::Sender(const string &host, unsigned short port)
    : m_socket(make_unique<Socket>()) {
  struct hostent *he(gethostbyname(host.c_str()));
  if (!he) {
    throw runtime_error("gethobyname failed for " + host);
  }

  struct sockaddr_in server;
  memcpy(&server.sin_addr, he->h_addr_list[0], he->h_length);
  server.sin_family = AF_INET;
  server.sin_port = htons(port);

  if (connect(m_socket->get_fd(), (struct sockaddr *)&server, sizeof(server)) <
      0) {
    throw runtime_error("Failed to connect.");
  }
}

bool Sender::send(const string &msg) {
  return ::send(m_socket->get_fd(), msg.c_str(), msg.size(), 0) >= 0;
}
