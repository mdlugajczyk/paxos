#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <memory>
#include <string>

class Socket {
public:
  Socket();
  Socket(int fd);
  int connect(const std::string &host, unsigned short port) const;
  int bind(unsigned short port) const;
  int listen() const;
  std::string recv() const;
  std::unique_ptr<Socket> accept() const;
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
  bool connect();
  std::string m_host;
  unsigned short m_port;
  bool m_connected;
  std::unique_ptr<Socket> m_socket;
};

#endif
