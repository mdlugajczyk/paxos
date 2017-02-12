#ifndef __DRIVER_H_
#define __DRIVER_H_

#include "paxos.h"
#include "socket.h"
#include <memory>
#include <string>
#include <vector>

struct Host {
  Host(const std::string &host, unsigned short port);
  std::string m_host;
  unsigned short m_port;
};

class PaxosDriver {
public:
  PaxosDriver(const std::vector<Host> &hosts, unsigned short port,
              const std::string &node_name, const std::string &data_dir,
              const std::string &value = "");
  void run();

private:
  void broadcast(const std::string &msg);
  unsigned int m_quorum_size;
  Paxos::Node m_paxos_node;
  Receiver m_receiver;
  std::string m_value;
  std::vector<std::unique_ptr<Sender>> m_senders;
};

#endif
