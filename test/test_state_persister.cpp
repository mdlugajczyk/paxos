#include "state_persister.h"
#include "gtest/gtest.h"
#include <iostream>
#include <stdio.h>
#include <unistd.h>

using namespace Paxos;

class StatePersisterTest : public ::testing::Test {
public:
  virtual void SetUp() { m_file_name = tmpnam(nullptr); }
  virtual void TearDown() { unlink(m_file_name.c_str()); }
  std::string m_file_name;
};

TEST_F(StatePersisterTest, NoStatePersisted) {
  StatePersister persister(m_file_name);
  const State s = persister.restore();
  ASSERT_EQ(s.m_value, "");
  ASSERT_EQ(s.m_proposal, ProposalID());
}

TEST_F(StatePersisterTest, PersistState) {
  const State s("a value", ProposalID("foo", 3));
  {
    StatePersister persister(m_file_name);
    persister.persist(s);
  }
  StatePersister persister(m_file_name);
  const State s2 = persister.restore();
  ASSERT_EQ(s2.m_value, s.m_value);
  ASSERT_EQ(s2.m_proposal, s.m_proposal);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
