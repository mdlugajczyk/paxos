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
  Proposer(const std::string &id, const int quorum_size);
  Message::PrepareMessage request_permission();
  std::experimental::optional<Message::PrepareMessage>
    process_noack(const Message::NoAck &noack);
  std::experimental::optional<Message::AcceptMessage>
    process_promise(const Message::PromiseMessage &noack);

private:
  const std::string m_node_id;
  const int m_quorum_size;
  ProposalID m_highest_proposal;
  ProposalID m_current_proposal;
  std::vector<NodeID> m_noack_senders;
  std::vector<NodeID> m_promise_senders;
};
}
#endif
