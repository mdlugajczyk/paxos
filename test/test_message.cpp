#include "message.h"
#include "gtest/gtest.h"

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

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
