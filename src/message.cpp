#include "message.h"

using namespace Paxos;
using namespace Paxos::Message;

Paxos::Message::Message::Message(const enum Type type) : m_type(type) {}

Paxos::Message::Message::~Message() {}

PermissionRequest::PermissionRequest(const ProposalID &id)
    : Message(Type::PermissionRequest), m_id(id) {}
