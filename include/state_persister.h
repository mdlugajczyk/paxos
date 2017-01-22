#ifndef __STATE_PERSISTER_H
#define __STATE_PERSISTER_H

#include <string>
#include "message.h"

namespace Paxos {
  struct State {
    State(const std::string &value, const ProposalID &proposal);
    std::string m_value;
    ProposalID m_proposal;
  };

  class StatePersister {
  public:
    StatePersister(const std::string &file);
    virtual ~StatePersister();
    virtual void persist(const State &state);
    virtual State restore();
  private:
    const std::string m_filename;
  };
}

#endif
