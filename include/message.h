#ifndef __MESSAGE_H_
#define __MESSAGE_H_

#include <string>

namespace Paxos {

typedef std::string NodeID;

struct ProposalID {
  ProposalID(const NodeID &node_id, const int proposal_id);
  NodeID m_node_id;
  int m_proposal_id;
};

namespace Message {

enum class Type { PermissionRequest };

class Message {
public:
  Message(const enum Type type);
  virtual ~Message();
  const enum Type m_type;
};

class PermissionRequest : public Message {
public:
  PermissionRequest(const ProposalID &id);
  const ProposalID m_id;
};
}
}
#endif
