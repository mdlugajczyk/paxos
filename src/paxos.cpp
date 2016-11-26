#include "paxos.h"

using namespace Paxos;

ProposalID::ProposalID(const std::string &node_id, const int proposal_id)
  : m_node_id(node_id), m_proposal_id(proposal_id) {}

PermissionRequestMessage::PermissionRequestMessage(const ProposalID &id)
  : m_id(id) {}

PermissionRequestMessage Proposer::request_permission() {
  return PermissionRequestMessage(ProposalID(m_node_id, 1));
}

Proposer::Proposer(const std::string &id)
  : m_node_id(id) {}
