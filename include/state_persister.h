#ifndef __STATE_PERSISTER_H
#define __STATE_PERSISTER_H

#include <string>
#include "message.h"

namespace Paxos {
  struct State {
    State(const std::string &value, const ProposalID &proposal);
    const std::string m_value;
    const ProposalID m_proposal;
  };

  class StatePersister {
  public:
    StatePersister(const std::string &file);
    virtual ~StatePersister();
    void persist(const State &state);
    State restore();
  private:
    const std::string m_filename;
  };
}

#endif
