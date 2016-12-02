#include "message.h"

using namespace Paxos;
using namespace Paxos::Message;

ProposalID::ProposalID(const NodeID &node_id, const int proposal_id)
    : m_node_id(node_id), m_proposal_id(proposal_id) {}

Paxos::Message::Message::Message(const enum Type type) : m_type(type) {}

Paxos::Message::Message::~Message() {}

PermissionRequest::PermissionRequest(const ProposalID &id)
    : Message(Type::PermissionRequest), m_id(id) {}
