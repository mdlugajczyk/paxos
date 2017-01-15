#include "message.h"
#include "gtest/gtest.h"
#include <sstream>

using namespace Paxos;

class MessageTest : public ::testing::Test {
public:
  virtual void SetUp() {}
};

TEST_F(MessageTest, CompareProposalIDs) {
  const ProposalID p1("b", 1);
  const ProposalID p2("a", 2);
  ASSERT_TRUE(p1 < p2);
}

TEST_F(MessageTest, IfIDsEqualCompareNodeIDs) {
  const ProposalID p1("b", 2);
  const ProposalID p2("a", 2);
  ASSERT_TRUE(p2 < p1);
}

TEST_F(MessageTest, CompareIDsDifferentNodes) {
  const ProposalID p1("b", 2);
  const ProposalID p2("a", 2);
  ASSERT_FALSE(p2 == p1);
  ASSERT_TRUE(p2 != p1);
}

TEST_F(MessageTest, CompareIDsDifferentProposals) {
  const ProposalID p1("a", 2);
  const ProposalID p2("a", 3);
  ASSERT_FALSE(p2 == p1);
  ASSERT_TRUE(p2 != p1);
}

TEST_F(MessageTest, CompareIdenticalProposals) {
  const ProposalID p1("a", 2);
  const ProposalID p2("a", 2);
  ASSERT_TRUE(p2 == p1);
  ASSERT_FALSE(p2 != p1);
}

TEST_F(MessageTest, SerializeProposalID) {
  const ProposalID proposal("foobar", 33);
  const std::string serialized = proposal.serialize();
  const ProposalID deserialized = ProposalID::deserialize(serialized);
  ASSERT_EQ(deserialized.m_node_id, proposal.m_node_id);
  ASSERT_EQ(deserialized.m_proposal_id, proposal.m_proposal_id);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
