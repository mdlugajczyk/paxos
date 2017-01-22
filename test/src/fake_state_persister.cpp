#include "fake_state_persister.h"

using namespace Paxos;
FakeStatePersister::FakeStatePersister(const State &initial_state)
    : StatePersister(""), m_state(initial_state), m_persist_calls(0),
      m_restore_calls(0) {}

void FakeStatePersister::persist(const State &state) {
  m_state = state;
  m_persist_calls++;
}

State FakeStatePersister::restore() {
  m_restore_calls++;
  return m_state;
}
