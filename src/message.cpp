#include "message.h"
#include <cstring>
#include <stdexcept>
#include <vector>

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

class Paxos::Message::Serializer {
public:
  std::string str() const { return std::string(&m_data[0], m_data.size()); };
  void serialize(int val) {
    serialize(reinterpret_cast<const char *>(&val), sizeof(val));
  }

  void serialize(const std::string &str) {
    serialize(str.size());
    serialize(str.c_str(), str.size());
  }

private:
  void serialize(const char *const val, int val_size) {
    const int current_size = m_data.size();
    m_data.resize(current_size + val_size);
    memcpy(&m_data[current_size], val, val_size);
  }
  std::vector<char> m_data;
};

class Deserializer {
public:
  Deserializer(const std::string &serialized_value)
      : m_index(0), m_data(serialized_value) {}

  template <typename T> T deserialize();

private:
  int m_index;
  const std::string m_data;
};

template <> int Deserializer::deserialize() {
  if (m_index + sizeof(int) > m_data.size())
    throw std::runtime_error("Failed to deserialize int.");
  int val;
  memcpy(&val, m_data.c_str() + m_index, sizeof(val));
  m_index += sizeof(val);
  return val;
}

template <> std::string Deserializer::deserialize() {
  const int length = deserialize<int>();
  if (m_index + length > m_data.size())
    throw std::runtime_error("Failed to deserialize string.");
  char buff[length];
  memcpy(buff, m_data.c_str() + m_index, length);
  m_index += length;
  return std::string(buff, length);
}

template <> ProposalID Deserializer::deserialize() {
  const auto str = deserialize<std::string>();
  return ProposalID::deserialize(str);
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
             const ProposalID &accepted_proposal)
    : Message(Type::NoAck, sender_id), m_rejected_proposal(rejected_proposal),
      m_accepted_proposal(accepted_proposal) {}

void NoAck::serialize_impl(Serializer &s) const {
  s.serialize(m_rejected_proposal.serialize());
  s.serialize(m_accepted_proposal.serialize());
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

std::unique_ptr<Paxos::Message::Message>
Paxos::Message::deserialize(const std::string &serialized_message) {
  Deserializer d(serialized_message);
  const Type type = static_cast<Type>(d.deserialize<int>());
  const std::string sender_id = d.deserialize<std::string>();
  switch (type) {
  case Type::Prepare: {
    const std::string value = d.deserialize<std::string>();
    ProposalID proposal = d.deserialize<ProposalID>();
    return std::make_unique<Paxos::Message::PrepareMessage>(proposal, value);
  }
  case Type::Promise: {
    const std::string value = d.deserialize<std::string>();
    ProposalID proposal = d.deserialize<ProposalID>();
    return std::make_unique<Paxos::Message::PromiseMessage>(proposal, sender_id,
                                                            value);
  }
  case Type::Accept: {
    const std::string value = d.deserialize<std::string>();
    ProposalID proposal = d.deserialize<ProposalID>();
    return std::make_unique<Paxos::Message::AcceptMessage>(proposal, value);
  }
  case Type::Accepted: {
    const std::string value = d.deserialize<std::string>();
    ProposalID proposal = d.deserialize<ProposalID>();
    return std::make_unique<Paxos::Message::AcceptedMessage>(proposal,
                                                             sender_id, value);
  }
  case Type::ConsensusReached: {
    const std::string value = d.deserialize<std::string>();
    return std::make_unique<Paxos::Message::ConsensusReached>(sender_id, value);
  }
  case Type::NoAck: {
    ProposalID rejected_proposal = d.deserialize<ProposalID>();
    ProposalID accepted_proposal = d.deserialize<ProposalID>();
    return std::make_unique<Paxos::Message::NoAck>(sender_id, rejected_proposal,
                                                   accepted_proposal);
  }
  }
}
