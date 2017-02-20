#include "driver.h"
#include "message.h"
#include <iostream>

using namespace Paxos::Message;
using namespace Paxos::Serialization;
using namespace std;
Host::Host(const string &host, unsigned short port)
    : m_host(host), m_port(port) {}

PaxosDriver::PaxosDriver(const vector<Host> &hosts, unsigned short port,
                         const string &node_name, const string &data_dir,
                         const string &value)
    : m_quorum_size((hosts.size() + 1) / 2 + 1),
      m_paxos_node(m_quorum_size, node_name, data_dir), m_receiver(port),
      m_value(value) {
  for (auto const &host : hosts)
    m_senders.push_back(make_unique<Sender>(host.m_host, host.m_port));
}

void PaxosDriver::run() {
  if (!m_value.empty()) {
    auto msg = m_paxos_node.propose_value(m_value);
    broadcast(msg->serialize());
  }

  while (1) {
    cout << "awaiting data" << endl;
    const auto data = m_receiver.recv();
    const auto msg = deserialize(data);
    cout << "received msg type " << static_cast<int>(msg->m_type) << endl;
    const auto response = m_paxos_node.process_message(msg);
    if (response) {
      cout << "responding with msg " << static_cast<int>(response->m_type)
           << endl;
      broadcast(response->serialize());
    }
  }
}

void PaxosDriver::broadcast(const string &msg) {
  int failures = 0;
  for (const auto &sender : m_senders) {
    if (!sender->send(msg))
      failures++;
  }

  if (failures >= m_quorum_size) {
    throw runtime_error("Message didn't reach the quorum.");
  }
}
