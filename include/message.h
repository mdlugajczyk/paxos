#ifndef __MESSAGE_H_
#define __MESSAGE_H_

#include <string>

namespace Paxos {

typedef std::string NodeID;

struct ProposalID {
  ProposalID(const NodeID &node_id, const int proposal_id);
  bool operator<(const ProposalID &other) const;
  bool operator==(const ProposalID &other) const;
  bool operator!=(const ProposalID &other) const;
  std::string serialize() const;
  static ProposalID deserialize(const std::string &str);
  NodeID m_node_id;
  int m_proposal_id;
};

namespace Message {

  enum class Type { Prepare, Promise, Accept, Accepted, ConsensusReached, NoAck };

class Message {
public:
  Message(const enum Type type, const NodeID &sender_id);
  virtual ~Message();
  const enum Type m_type;
  const NodeID m_sender_id;
};

class PrepareMessage : public Message {
public:
  PrepareMessage(const ProposalID &id, const std::string &value);
  const std::string m_value;
  const ProposalID m_proposal_id;
};

class PromiseMessage : public Message {
public:
  PromiseMessage(const ProposalID &id, const NodeID &sender_id,
                 const std::string &value);
  const std::string m_value;
  const ProposalID m_proposal_id;
};

class AcceptMessage : public Message {
public:
  AcceptMessage(const ProposalID &id, const std::string &value);
  const std::string m_value;
  const ProposalID m_proposal_id;
};

class AcceptedMessage : public Message {
public:
  AcceptedMessage(const ProposalID &id, const std::string &sender_id,
                  const std::string &value);
  const std::string m_value;
  const ProposalID m_proposal_id;
};

class ConsensusReached : public Message {
public:
  ConsensusReached(const NodeID &sender_id, const std::string &value);
  const std::string m_value;
};

class NoAck : public Message {
public:
  NoAck(const NodeID &sender_id, const ProposalID &rejected_proposal,
        const ProposalID &accepted_proposal);
  const ProposalID m_rejected_proposal;
  const ProposalID m_accepted_proposal;
};
}
}
#endif
