#include "paxos.h"

using namespace Paxos;
using namespace std::experimental;

QuorumTooSmallException::QuorumTooSmallException()
    : std::runtime_error("Quorum size can't be smaller than 2.") {}

Message::PrepareMessage Proposer::request_permission() {
  const ProposalID id(m_node_id, m_highest_proposal.m_proposal_id + 1);
  m_highest_proposal = id;
  m_current_proposal = id;
  return Message::PrepareMessage(id);
}

optional<Message::PrepareMessage>
Proposer::process_noack(const Message::NoAck &noack) {
  if (m_highest_proposal < noack.m_accepted_proposal)
    m_highest_proposal = noack.m_accepted_proposal;
  if (m_current_proposal != noack.m_proposal_id)
    return {};
  m_noack_senders.push_back(noack.m_sender_id);
  if (m_noack_senders.size() >= m_quorum_size)
    return request_permission();
  return {};
}

optional<Message::AcceptMessage>
Proposer::process_promise(const Message::PromiseMessage &promise) {
  if (m_highest_proposal < promise.m_proposal_id)
    m_highest_proposal = promise.m_proposal_id;
  if (promise.m_proposal_id != m_current_proposal)
    return {};
  m_promise_senders.push_back(promise.m_sender_id);
  if (m_promise_senders.size() >= m_quorum_size)
    return Message::AcceptMessage(m_current_proposal);
  return {};
}

Proposer::Proposer(const std::string &id, const int quorum_size,
                   const std::string &value)
    : m_node_id(id), m_quorum_size(quorum_size), m_value(value),
      m_highest_proposal(m_node_id, 0), m_current_proposal(m_node_id, 0) {
  if (m_quorum_size < 2) {
    throw QuorumTooSmallException();
  }
}

Acceptor::Acceptor(const std::string &id)
    : m_node_id(id), m_highest_proposal(m_node_id, 0) {}

std::unique_ptr<Message::Message>
Acceptor::process_prepare(const Message::PrepareMessage &prepare) {
  if (m_highest_proposal < prepare.m_proposal_id) {
    m_highest_proposal = prepare.m_proposal_id;
    return std::make_unique<Message::PromiseMessage>(prepare.m_proposal_id,
                                                     m_node_id);
  }

  return std::make_unique<Message::NoAck>(m_node_id, prepare.m_proposal_id,
                                          m_highest_proposal);
}
