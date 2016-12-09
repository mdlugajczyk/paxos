#ifndef __PAXOS_H__
#define __PAXOS_H__

#include "message.h"
#include <experimental/optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace Paxos {
class QuorumTooSmallException : public std::runtime_error {
public:
  QuorumTooSmallException();
};

class Proposer {
public:
  explicit Proposer(const std::string &id, const int quorum_size);
  Message::PermissionRequest request_permission();
  std::experimental::optional<Message::PermissionRequest>
  process_noack(const Message::NoAck &noack);

private:
  const std::string m_node_id;
  const int m_quorum_size;
  ProposalID m_highest_proposal;
  std::vector<NodeID> m_noack_senders;
};
}
#endif
