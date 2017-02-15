#ifndef __MESSAGE_H_
#define __MESSAGE_H_

#include <memory>
#include <string>

namespace Paxos {

typedef std::string NodeID;

struct ProposalID {
  ProposalID();
  ProposalID(const NodeID &node_id, const int proposal_id);
  bool operator<(const ProposalID &other) const;
  bool operator==(const ProposalID &other) const;
  bool operator!=(const ProposalID &other) const;
  std::string serialize() const;
  static ProposalID deserialize(const std::string &str);
  NodeID m_node_id;
  int m_proposal_id;
};

namespace Persistence {
  class Serializer;
}

namespace Message {

enum class Type { Prepare, Promise, Accept, Accepted, ConsensusReached, NoAck };

class Message {
public:
  Message(const enum Type type, const NodeID &sender_id);
  virtual ~Message();
  std::string serialize() const;
  const enum Type m_type;
  const NodeID m_sender_id;

private:
  virtual void serialize_impl(Persistence::Serializer &serializer) const = 0;
};

std::shared_ptr<Message> deserialize(const std::string &serialized_msg);

class PrepareMessage : public Message {
public:
  PrepareMessage(const ProposalID &id, const std::string &value);
  const std::string m_value;
  const ProposalID m_proposal_id;

private:
  void serialize_impl(Persistence::Serializer &serializer) const;
};

class PromiseMessage : public Message {
public:
  PromiseMessage(const ProposalID &id, const NodeID &sender_id,
                 const std::string &value);
  const std::string m_value;
  const ProposalID m_proposal_id;

private:
  void serialize_impl(Persistence::Serializer &serializer) const;
};

class AcceptMessage : public Message {
public:
  AcceptMessage(const ProposalID &id, const std::string &value);
  const std::string m_value;
  const ProposalID m_proposal_id;

private:
  void serialize_impl(Persistence::Serializer &serializer) const;
};

class AcceptedMessage : public Message {
public:
  AcceptedMessage(const ProposalID &id, const std::string &sender_id,
                  const std::string &value);
  const std::string m_value;
  const ProposalID m_proposal_id;

private:
  void serialize_impl(Persistence::Serializer &serializer) const;
};

class ConsensusReached : public Message {
public:
  ConsensusReached(const NodeID &sender_id, const std::string &value);
  const std::string m_value;

private:
  void serialize_impl(Persistence::Serializer &serializer) const;
};

class NoAck : public Message {
public:
  NoAck(const NodeID &sender_id, const ProposalID &rejected_proposal,
        const ProposalID &accepted_proposal, const std::string &accepted_value);
  const ProposalID m_rejected_proposal;
  const ProposalID m_accepted_proposal;
  const std::string m_accepted_value;

private:
  void serialize_impl(Persistence::Serializer &serializer) const;
};
}
}
#endif
