#ifndef __PAXOS_H__
#define __PAXOS_H__

#include <string>
#include <stdexcept>
#include "message.h"

namespace Paxos {
  class QuorumTooSmallException : public std::runtime_error {
  public:
    QuorumTooSmallException();
  };

  class Proposer {
  public:
    explicit Proposer(const std::string &id, const int quorum_size);
    Message::PermissionRequest request_permission();
  private:
    const std::string m_node_id;
    const int m_quorum_size;
    ProposalID m_highest_proposal;
  };
}
#endif
