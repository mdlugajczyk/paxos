#ifndef __MESSAGE_H_
#define __MESSAGE_H_

#include <string>

namespace Paxos {

struct ProposalID {
  ProposalID(const std::string &node_id, const int proposal_id);
  std::string m_node_id;
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
