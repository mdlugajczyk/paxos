#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <memory>
#include <string>

class Socket {
public:
  Socket();
  Socket(int fd);
  ~Socket();
  int get_fd() const;

private:
  int m_fd;
};

class Receiver {
public:
  explicit Receiver(unsigned short port);
  virtual std::string recv();

private:
  std::unique_ptr<Socket> m_socket;
};

class Sender {
public:
  Sender(const std::string &host, unsigned short port);
  virtual bool send(const std::string &msg);

private:
  std::unique_ptr<Socket> m_socket;
};

#endif
