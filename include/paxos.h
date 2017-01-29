#ifndef __PAXOS_H__
#define __PAXOS_H__

#include "message.h"
#include <map>
#include <set>
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
  std::shared_ptr<Message::PrepareMessage>
  request_permission(const std::string &value);
  std::shared_ptr<Message::PrepareMessage>
  process_noack(const Message::NoAck &noack);
  std::shared_ptr<Message::AcceptMessage>
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

  std::shared_ptr<Message::Message>
  process_prepare(const Message::PrepareMessage &msg);

  std::shared_ptr<Message::Message>
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

  std::shared_ptr<Message::ConsensusReached>
  process_accepted(const Message::AcceptedMessage &msg);

private:
  const std::string m_id;
  const int m_quorum_size;
  std::map<ProposalID, std::set<NodeID>> m_accepted_proposals;
  std::map<NodeID, ProposalID> m_acceptors;
};

class Node {
public:
  Node(int quorum_size, const std::string &node_name,
       const std::string &data_directory);
  std::shared_ptr<Message::Message>
  process_message(std::shared_ptr<Message::Message> msg);

private:
  Proposer m_proposer;
  Acceptor m_acceptor;
  Learner m_learner;
};
}
#endif
