#include "paxos.h"

using namespace Paxos;
using namespace std::experimental;

QuorumTooSmallException::QuorumTooSmallException()
    : std::runtime_error("Quorum size can't be smaller than 2.") {}

Message::PermissionRequest Proposer::request_permission() {
  const ProposalID id(m_node_id, m_highest_proposal.m_proposal_id + 1);
  m_highest_proposal = id;
  return Message::PermissionRequest(id);
}

optional<Message::PermissionRequest>
Proposer::process_noack(const Message::NoAck &noack) {
  if (m_highest_proposal < noack.m_accepted_proposal)
    m_highest_proposal = noack.m_accepted_proposal;
  m_noack_senders.push_back(noack.m_sender_id);
  if (m_noack_senders.size() >= m_quorum_size)
    return request_permission();
  return {};
}

Proposer::Proposer(const std::string &id, const int quorum_size)
    : m_node_id(id), m_quorum_size(quorum_size),
      m_highest_proposal(m_node_id, 0) {
  if (m_quorum_size < 2) {
    throw QuorumTooSmallException();
  }
}
