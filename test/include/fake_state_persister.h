#ifndef __FAKE_STATE_PERSISTER_H_
#define __FAKE_STATE_PERSISTER_H_

#include "state_persister.h"

class FakeStatePersister : public Paxos::StatePersister {
public:
  FakeStatePersister(const Paxos::State &initial_state);
  virtual void persist(const Paxos::State &state);
  virtual Paxos::State restore();
  Paxos::State m_state;
};

#endif
