#include "state_persister.h"
#include "persistence.h"
#include <cstdio>
#include <unistd.h>

using namespace Paxos;

State::State(const std::string &value, const ProposalID &proposal)
    : m_value(value), m_proposal(proposal) {}

StatePersister::StatePersister(const std::string &file) : m_filename(file) {}

StatePersister::~StatePersister() {}
void StatePersister::persist(const State &state) {
  Persistence::Serializer s;
  s.serialize(state.m_value);
  s.serialize(state.m_proposal.serialize());
  const std::string serialized_data = s.str();
  FILE *f = fopen(m_filename.c_str(), "wb");
  fwrite(serialized_data.c_str(), serialized_data.size(), 1, f);
  fsync(fileno(f));
  fclose(f);
}

State StatePersister::restore() {
  FILE *f = fopen(m_filename.c_str(), "rb");
  if (!f)
    return State("", ProposalID());
  fseek(f, 0, SEEK_END);
  const int size = ftell(f);
  rewind(f);
  char buffer[size];
  fread(buffer, 1, size, f);
  fclose(f);
  Persistence::Deserializer d(std::string(buffer, size));
  try {
    const std::string value = d.deserialize<std::string>();
    const ProposalID proposal = d.deserialize<ProposalID>();
    return State(value, proposal);
  } catch (const std::runtime_error &e) {
    return State("", ProposalID());
  }
}
