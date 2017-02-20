#include "message.h"
#include "serialization.h"

using namespace Paxos;
using namespace Paxos::Message;
using namespace Paxos::Persistence;

ProposalID::ProposalID(const NodeID &node_id, const int proposal_id)
    : m_node_id(node_id), m_proposal_id(proposal_id) {}

ProposalID::ProposalID() : m_node_id(""), m_proposal_id(0) {}

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

std::string ProposalID::serialize() const {
  Serializer s;
  s.serialize(m_proposal_id);
  s.serialize(m_node_id);
  return s.str();
}

ProposalID ProposalID::deserialize(const std::string &serialized) {
  Deserializer d(serialized);
  const int proposal_id = d.deserialize<int>();
  const std::string node_id = d.deserialize<std::string>();
  return ProposalID(node_id, proposal_id);
}

template <> ProposalID Deserializer::deserialize() {
  const auto str = deserialize<std::string>();
  return ProposalID::deserialize(str);
}

Paxos::Message::Message::Message(const enum Type type, const NodeID &sender_id)
    : m_type(type), m_sender_id(sender_id) {}

Paxos::Message::Message::~Message() {}

std::string Paxos::Message::Message::serialize() const {
  Serializer s;
  s.serialize(static_cast<int>(m_type));
  s.serialize(m_sender_id);
  serialize_impl(s);
  return s.str();
}

PrepareMessage::PrepareMessage(const ProposalID &id, const std::string &value)
    : Message(Type::Prepare, id.m_node_id), m_value(value), m_proposal_id(id) {}

void PrepareMessage::serialize_impl(Serializer &s) const {
  s.serialize(m_value);
  s.serialize(m_proposal_id.serialize());
}

NoAck::NoAck(const NodeID &sender_id, const ProposalID &rejected_proposal,
             const ProposalID &accepted_proposal,
             const std::string &accepted_value)
    : Message(Type::NoAck, sender_id), m_rejected_proposal(rejected_proposal),
      m_accepted_proposal(accepted_proposal), m_accepted_value(accepted_value) {
}

void NoAck::serialize_impl(Serializer &s) const {
  s.serialize(m_rejected_proposal.serialize());
  s.serialize(m_accepted_proposal.serialize());
  s.serialize(m_accepted_value);
}

PromiseMessage::PromiseMessage(const ProposalID &id, const NodeID &node_id,
                               const std::string &value)
    : Message(Type::Promise, node_id), m_value(value), m_proposal_id(id) {}

void PromiseMessage::serialize_impl(Serializer &s) const {
  s.serialize(m_value);
  s.serialize(m_proposal_id.serialize());
}

AcceptMessage::AcceptMessage(const ProposalID &id, const std::string &value)
    : Message(Type::Accept, id.m_node_id), m_value(value), m_proposal_id(id) {}

void AcceptMessage::serialize_impl(Serializer &s) const {
  s.serialize(m_value);
  s.serialize(m_proposal_id.serialize());
}

AcceptedMessage::AcceptedMessage(const ProposalID &id,
                                 const std::string &sender_id,
                                 const std::string &value)
    : Message(Type::Accepted, sender_id), m_value(value), m_proposal_id(id) {}

void AcceptedMessage::serialize_impl(Serializer &s) const {
  s.serialize(m_value);
  s.serialize(m_proposal_id.serialize());
}

ConsensusReached::ConsensusReached(const NodeID &sender_id,
                                   const std::string &value)
    : Message(Type::ConsensusReached, sender_id), m_value(value) {}

void ConsensusReached::serialize_impl(Serializer &s) const {
  s.serialize(m_value);
}

std::shared_ptr<Paxos::Message::Message>
Paxos::Message::deserialize(const std::string &serialized_message) {
  Deserializer d(serialized_message);
  const Type type = static_cast<Type>(d.deserialize<int>());
  const std::string sender_id = d.deserialize<std::string>();

  if (type == Type::NoAck) {
    ProposalID rejected_proposal = d.deserialize<ProposalID>();
    ProposalID accepted_proposal = d.deserialize<ProposalID>();
    const std::string accepted_value = d.deserialize<std::string>();
    return std::make_shared<Paxos::Message::NoAck>(
        sender_id, rejected_proposal, accepted_proposal, accepted_value);
  }

  const std::string value = d.deserialize<std::string>();

  if (type == Type::ConsensusReached) {
    return std::make_shared<Paxos::Message::ConsensusReached>(sender_id, value);
  }

  const ProposalID proposal = d.deserialize<ProposalID>();
  switch (type) {
  case Type::Prepare:
    return std::make_shared<Paxos::Message::PrepareMessage>(proposal, value);
  case Type::Promise:
    return std::make_shared<Paxos::Message::PromiseMessage>(proposal, sender_id,
                                                            value);
  case Type::Accept:
    return std::make_shared<Paxos::Message::AcceptMessage>(proposal, value);
  case Type::Accepted:
    return std::make_shared<Paxos::Message::AcceptedMessage>(proposal,
                                                             sender_id, value);
  case Type::NoAck:
  case Type::ConsensusReached:
    return {}; // Won't get there
  }
}
