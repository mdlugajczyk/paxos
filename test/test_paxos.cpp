#include "paxos.h"
#include "gtest/gtest.h"
#include <cstdint>
#include <iostream>

using namespace Paxos;

class PaxosTest : public ::testing::Test {
public:
  virtual void SetUp() {}
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

TEST_F(PaxosTest, ReceiveLessThanQuorumRejectedMessagesAfterProposal) {
  Proposer p("foo", 2);
  const auto permission_msg = p.request_permission();
  const Message::NoAck noack("bar", permission_msg.m_id, ProposalID("baz", 2));
  const auto nack_response = p.process_noack(noack);
  ASSERT_FALSE(nack_response);
}

TEST_F(PaxosTest, IfQuorumRejectedProposalShouldSendNew) {
  Proposer p("foo", 2);
  const auto permission_msg = p.request_permission();
  const ProposalID accepted_proposal("baz", 3);
  const Message::NoAck noack1("bar", permission_msg.m_id, accepted_proposal);
  p.process_noack(noack1);
  const Message::NoAck noack2("baz", permission_msg.m_id, accepted_proposal);
  const auto noack_response = p.process_noack(noack2);
  ASSERT_TRUE(noack_response);
  ASSERT_EQ(noack_response->m_sender_id, "foo");
  ASSERT_EQ(noack_response->m_id.m_node_id, "foo");
  ASSERT_EQ(noack_response->m_id.m_proposal_id, 4);
}

TEST_F(PaxosTest, IgnoreQuorumRejectingSomeoneElseProposal) {
  Proposer p("foo", 2);
  const auto permission_msg = p.request_permission();
  const ProposalID accepted_proposal("baz", 3);
  const ProposalID rejected_proposal("bar", 4);
  const Message::NoAck noack1("bar", rejected_proposal, accepted_proposal);
  p.process_noack(noack1);
  const Message::NoAck noack2("baz", rejected_proposal, accepted_proposal);
  const auto noack_response = p.process_noack(noack2);
  ASSERT_FALSE(noack_response);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
