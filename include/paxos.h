#ifndef __PAXOS_H__
#define __PAXOS_H__

#include "message.h"
#include <experimental/optional>
#include <stdexcept>
#include <string>
#include <vector>
#include <map>
#include <set>

namespace Paxos {
class QuorumTooSmallException : public std::runtime_error {
public:
  QuorumTooSmallException();
};

class Proposer {
public:
  Proposer(const std::string &id, const int quorum_size,
           const std::string &value);
  Message::PrepareMessage request_permission();
  std::experimental::optional<Message::PrepareMessage>
  process_noack(const Message::NoAck &noack);
  std::experimental::optional<Message::AcceptMessage>
  process_promise(const Message::PromiseMessage &noack);

private:
  const std::string m_node_id;
  const int m_quorum_size;
  std::string m_value;
  ProposalID m_highest_proposal;
  ProposalID m_current_proposal;
  std::vector<NodeID> m_noack_senders;
  std::vector<NodeID> m_promise_senders;
};

class StatePersister;

class Acceptor {
public:
  Acceptor(const std::string &id, std::shared_ptr<StatePersister> persister);

  std::unique_ptr<Message::Message>
  process_prepare(const Message::PrepareMessage &msg);

  std::unique_ptr<Message::Message>
  process_accept(const Message::AcceptMessage &msg);

private:
  std::shared_ptr<StatePersister> m_state_persister;
  const std::string m_node_id;
  ProposalID m_highest_proposal;
  std::string m_value;
};

class Learner {
public:
  Learner(const std::string &id, const int quorum_size);

  std::experimental::optional<Message::ConsensusReached>
    process_accepted(const Message::AcceptedMessage &msg);
private:
  const std::string m_id;
  const int m_quorum_size;
  std::map<ProposalID, std::set<NodeID>> m_accepted_proposals;
  std::map<NodeID, ProposalID> m_acceptors;
};
}
#endif
