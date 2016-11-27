#include "paxos.h"

using namespace Paxos;

ProposalID::ProposalID(const std::string &node_id, const int proposal_id)
  : m_node_id(node_id), m_proposal_id(proposal_id) {}

PermissionRequestMessage::PermissionRequestMessage(const ProposalID &id)
  : m_id(id) {}

PermissionRequestMessage Proposer::request_permission() {
  const ProposalID id(m_node_id, m_highest_proposal.m_proposal_id + 1);
  m_highest_proposal = id;
  return PermissionRequestMessage(id);
}

Proposer::Proposer(const std::string &id)
  : m_node_id(id), m_highest_proposal(m_node_id, 0) {}
