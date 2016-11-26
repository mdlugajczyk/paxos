#include <cstdint>
#include "gtest/gtest.h"
#include <iostream>
#include "paxos.h"

using namespace Paxos;

class PaxosTest : public ::testing::Test {
public:
  virtual void SetUp() {
  }
};

TEST_F(PaxosTest, ProposerRequestPermission) {
  Proposer p("foo");
  const auto msg = p.request_permission();
  ASSERT_EQ(msg.m_id.m_node_id, "foo");
  ASSERT_EQ(msg.m_id.m_proposal_id, 1);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
