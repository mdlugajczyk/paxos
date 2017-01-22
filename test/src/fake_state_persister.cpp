#include "fake_state_persister.h"

using namespace Paxos;
FakeStatePersister::FakeStatePersister(const State &initial_state)
    : StatePersister(""), m_state(initial_state) {}

void FakeStatePersister::persist(const State &state) { m_state = state; }

State FakeStatePersister::restore() { return m_state; }
