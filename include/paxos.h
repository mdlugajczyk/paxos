#ifndef __PAXOS_H__
#define __PAXOS_H__

#include <string>
#include <stdexcept>

namespace Paxos {
  class QuorumTooSmallException : public std::runtime_error {
  public:
    QuorumTooSmallException();
  };

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
    explicit Proposer(const std::string &id, const int quorum_size);
    PermissionRequestMessage request_permission();
  private:
    const std::string m_node_id;
    const int m_quorum_size;
    ProposalID m_highest_proposal;
  };
}

#endif
