#ifndef __MESSAGE_H_
#define __MESSAGE_H_

#include <string>

namespace Paxos {

typedef std::string NodeID;

struct ProposalID {
  ProposalID(const NodeID &node_id, const int proposal_id);
  bool operator<(const ProposalID &other) const;
  NodeID m_node_id;
  int m_proposal_id;
};

namespace Message {

enum class Type { PermissionRequest, NoAck };

class Message {
public:
  Message(const enum Type type, const NodeID &sender_id);
  virtual ~Message();
  const enum Type m_type;
  const NodeID m_sender_id;
};

class PermissionRequest : public Message {
public:
  PermissionRequest(const ProposalID &id);
  const ProposalID m_id;
};

class NoAck : public Message {
public:
  NoAck(const NodeID &sender_id, const ProposalID &rejected_proposal,
        const ProposalID &accepted_proposal);
  const ProposalID &m_rejected_proposal;
  const ProposalID &m_accepted_proposal;
};
}
}
#endif
