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
  Proposer p("foo", 2);
  const auto msg = p.request_permission();
  ASSERT_EQ(msg.m_id.m_node_id, "foo");
  ASSERT_EQ(msg.m_id.m_proposal_id, 1);
}

TEST_F(PaxosTest, ProposerNewPermissionRequestNewID) {
  Proposer p("foo", 2);
  const auto first_msg = p.request_permission();
  const auto second_msg = p.request_permission();
  ASSERT_EQ(first_msg.m_id.m_proposal_id, 1);
  ASSERT_EQ(second_msg.m_id.m_proposal_id, 2);
}

TEST_F(PaxosTest, ProposerCantBeCreatedWithQuorumBelow2) {
  ASSERT_THROW(Proposer("foo", 1), QuorumTooSmallException);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
