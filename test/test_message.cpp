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

TEST_F(MessageTest, DefaultProposal) {
  ProposalID p;
  ASSERT_EQ(p.m_node_id, "");
  ASSERT_EQ(p.m_proposal_id, 0);
}

TEST_F(MessageTest, SerializeProposalID) {
  const ProposalID proposal("foobar", 33);
  const std::string serialized = proposal.serialize();
  const ProposalID deserialized = ProposalID::deserialize(serialized);
  ASSERT_EQ(deserialized.m_node_id, proposal.m_node_id);
  ASSERT_EQ(deserialized.m_proposal_id, proposal.m_proposal_id);
}

TEST_F(MessageTest, SerializePrepareMessage) {
  const Message::PrepareMessage msg(ProposalID("foo", 3), "test value 123");
  const std::string serialized = msg.serialize();
  const auto deserialized = Message::deserialize(serialized);
  ASSERT_EQ(msg.m_type, deserialized->m_type);
  const auto prepare_msg =
      dynamic_cast<Message::PrepareMessage &>(*deserialized);
  ASSERT_EQ(msg.m_sender_id, prepare_msg.m_sender_id);
  ASSERT_EQ(msg.m_value, prepare_msg.m_value);
  ASSERT_EQ(msg.m_proposal_id, prepare_msg.m_proposal_id);
}

TEST_F(MessageTest, SerializePromiseMessage) {
  const Message::PromiseMessage msg(ProposalID("foo", 3), "sender",
                                    "test value 123");
  const std::string serialized = msg.serialize();
  const auto deserialized = Message::deserialize(serialized);
  ASSERT_EQ(msg.m_type, deserialized->m_type);
  const auto prepare_msg =
      dynamic_cast<Message::PromiseMessage &>(*deserialized);
  ASSERT_EQ(msg.m_sender_id, prepare_msg.m_sender_id);
  ASSERT_EQ(msg.m_value, prepare_msg.m_value);
  ASSERT_EQ(msg.m_proposal_id, prepare_msg.m_proposal_id);
}

TEST_F(MessageTest, SerializeAcceptMessage) {
  const Message::AcceptMessage msg(ProposalID("foo", 3), "test value 123");
  const std::string serialized = msg.serialize();
  const auto deserialized = Message::deserialize(serialized);
  ASSERT_EQ(msg.m_type, deserialized->m_type);
  const auto prepare_msg =
      dynamic_cast<Message::AcceptMessage &>(*deserialized);
  ASSERT_EQ(msg.m_sender_id, prepare_msg.m_sender_id);
  ASSERT_EQ(msg.m_value, prepare_msg.m_value);
  ASSERT_EQ(msg.m_proposal_id, prepare_msg.m_proposal_id);
}

TEST_F(MessageTest, SerializeAcceptedMessage) {
  const Message::AcceptedMessage msg(ProposalID("foo", 3), "sender",
                                     "test value 123");
  const std::string serialized = msg.serialize();
  const auto deserialized = Message::deserialize(serialized);
  ASSERT_EQ(msg.m_type, deserialized->m_type);
  const auto prepare_msg =
      dynamic_cast<Message::AcceptedMessage &>(*deserialized);
  ASSERT_EQ(msg.m_sender_id, prepare_msg.m_sender_id);
  ASSERT_EQ(msg.m_value, prepare_msg.m_value);
  ASSERT_EQ(msg.m_proposal_id, prepare_msg.m_proposal_id);
}

TEST_F(MessageTest, SerializeConsensusReachedMessage) {
  const Message::ConsensusReached msg("sender", "test value 123");
  const std::string serialized = msg.serialize();
  const auto deserialized = Message::deserialize(serialized);
  ASSERT_EQ(msg.m_type, deserialized->m_type);
  const auto prepare_msg =
      dynamic_cast<Message::ConsensusReached &>(*deserialized);
  ASSERT_EQ(msg.m_sender_id, prepare_msg.m_sender_id);
  ASSERT_EQ(msg.m_value, prepare_msg.m_value);
}

TEST_F(MessageTest, SerializeNoAckMessage) {
  const Message::NoAck msg("sender", ProposalID("foo", 3), ProposalID("bar", 4),
                           "foo");
  const std::string serialized = msg.serialize();
  const auto deserialized = Message::deserialize(serialized);
  ASSERT_EQ(msg.m_type, deserialized->m_type);
  const auto noack_msg = dynamic_cast<Message::NoAck &>(*deserialized);
  ASSERT_EQ(msg.m_sender_id, noack_msg.m_sender_id);
  ASSERT_EQ(msg.m_rejected_proposal, noack_msg.m_rejected_proposal);
  ASSERT_EQ(msg.m_accepted_proposal, noack_msg.m_accepted_proposal);
  ASSERT_EQ(msg.m_accepted_value, noack_msg.m_accepted_value);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
