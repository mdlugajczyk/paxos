#include "driver.h"
#include "message.h"

using namespace Paxos::Message;
using namespace Paxos::Persistence;
using namespace std;
Host::Host(const string &host, unsigned short port)
    : m_host(host), m_port(port) {}

PaxosDriver::PaxosDriver(const vector<Host> &hosts, unsigned short port,
                         const string &node_name, const string &data_dir,
                         const string &value)
    : m_paxos_node(hosts.size() / 2 + 1, node_name, data_dir), m_receiver(port),
      m_value(value) {
  for (auto const &host : hosts)
    m_senders.push_back(make_unique<Sender>(host.m_host, host.m_port));
}

void PaxosDriver::run() {
  if (!m_value.empty()) {
  }

  //  Serializer serializer;
  while (1) {
    const std::string response = m_receiver.recv();
    std::unique_ptr<Message> msg = deserialize(response);
  }
}
