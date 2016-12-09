#include "message.h"

using namespace Paxos;
using namespace Paxos::Message;

ProposalID::ProposalID(const NodeID &node_id, const int proposal_id)
    : m_node_id(node_id), m_proposal_id(proposal_id) {}

bool ProposalID::operator<(const Paxos::ProposalID &other) const {
  if (m_proposal_id == other.m_proposal_id)
    return m_node_id < other.m_node_id;
  return m_proposal_id < other.m_proposal_id;
}

bool ProposalID::operator==(const Paxos::ProposalID &other) const {
  return m_node_id == other.m_node_id && m_proposal_id == other.m_proposal_id;
}

bool ProposalID::operator!=(const Paxos::ProposalID &other) const {
  return !operator==(other);
}

Paxos::Message::Message::Message(const enum Type type, const NodeID &sender_id)
    : m_type(type), m_sender_id(sender_id) {}

Paxos::Message::Message::~Message() {}

PermissionRequest::PermissionRequest(const ProposalID &id)
    : Message(Type::PermissionRequest, id.m_node_id), m_id(id) {}

NoAck::NoAck(const NodeID &sender_id, const ProposalID &rejected_proposal,
             const ProposalID &accepted_proposal)
    : Message(Type::NoAck, sender_id), m_rejected_proposal(rejected_proposal),
      m_accepted_proposal(accepted_proposal) {}
