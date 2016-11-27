#ifndef __PAXOS_H__
#define __PAXOS_H__

#include <string>

namespace Paxos {
  struct ProposalID {
    ProposalID(const std::string &node_id, const int proposal_id);
    std::string m_node_id;
    int m_proposal_id;
  };

  struct PermissionRequestMessage {
    PermissionRequestMessage(const ProposalID &id);
    const ProposalID m_id;
  };

  class Proposer {
  public:
    explicit Proposer(const std::string &id);
    PermissionRequestMessage request_permission();
  private:
    const std::string m_node_id;
    ProposalID m_highest_proposal;
  };
}

#endif
