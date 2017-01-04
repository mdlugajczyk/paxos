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
  Proposer p("foo", 2, "value");
  const auto msg = p.request_permission();
  ASSERT_EQ(msg.m_proposal_id.m_node_id, "foo");
  ASSERT_EQ(msg.m_proposal_id.m_proposal_id, 1);
  ASSERT_EQ(msg.m_value, "value");
}

TEST_F(PaxosTest, ProposerNewPermissionRequestNewID) {
  Proposer p("foo", 2, "value");
  const auto first_msg = p.request_permission();
  const auto second_msg = p.request_permission();
  ASSERT_EQ(first_msg.m_proposal_id.m_proposal_id, 1);
  ASSERT_EQ(second_msg.m_proposal_id.m_proposal_id, 2);
}

TEST_F(PaxosTest, ProposerCantBeCreatedWithQuorumBelow2) {
  ASSERT_THROW(Proposer("foo", 1, "value"), QuorumTooSmallException);
}

TEST_F(PaxosTest, ReceiveLessThanQuorumRejectedMessagesAfterProposal) {
  Proposer p("foo", 2, "value");
  const auto permission_msg = p.request_permission();
  const Message::NoAck noack("bar", permission_msg.m_proposal_id,
                             ProposalID("baz", 2));
  const auto nack_response = p.process_noack(noack);
  ASSERT_FALSE(nack_response);
}

TEST_F(PaxosTest, IfQuorumRejectedProposalShouldSendNew) {
  Proposer p("foo", 2, "value");
  const auto permission_msg = p.request_permission();
  const ProposalID accepted_proposal("baz", 3);
  const Message::NoAck noack1("bar", permission_msg.m_proposal_id,
                              accepted_proposal);
  p.process_noack(noack1);
  const Message::NoAck noack2("baz", permission_msg.m_proposal_id,
                              accepted_proposal);
  const auto noack_response = p.process_noack(noack2);
  ASSERT_TRUE(noack_response);
  ASSERT_EQ(noack_response->m_sender_id, "foo");
  ASSERT_EQ(noack_response->m_proposal_id.m_node_id, "foo");
  ASSERT_EQ(noack_response->m_proposal_id.m_proposal_id, 4);
  ASSERT_EQ(noack_response->m_value, "value");
}

TEST_F(PaxosTest, IgnoreQuorumRejectingSomeoneElseProposal) {
  Proposer p("foo", 2, "value");
  const auto permission_msg = p.request_permission();
  const ProposalID accepted_proposal("baz", 3);
  const ProposalID rejected_proposal("bar", 4);
  const Message::NoAck noack1("bar", rejected_proposal, accepted_proposal);
  p.process_noack(noack1);
  const Message::NoAck noack2("baz", rejected_proposal, accepted_proposal);
  const auto noack_response = p.process_noack(noack2);
  ASSERT_FALSE(noack_response);
}

TEST_F(PaxosTest, CheckNoAcksForHighestProposalID) {
  Proposer p("foo", 2, "value");
  p.request_permission();
  const ProposalID accepted_proposal("bar", 4);
  const Message::NoAck noack("bar", ProposalID("baz", 3), accepted_proposal);
  p.process_noack(noack);
  const auto permission_request = p.request_permission();
  ASSERT_EQ(permission_request.m_proposal_id,
            ProposalID("foo", accepted_proposal.m_proposal_id + 1));
}

TEST_F(PaxosTest, DontActOnPromisesUntilQuorumIsReached) {
  Proposer p("foo", 2, "value");
  const auto prepare_msg = p.request_permission();
  const auto response = p.process_promise(
      Message::PromiseMessage(prepare_msg.m_proposal_id, "bar", ""));
  ASSERT_FALSE(response);
}

TEST_F(PaxosTest, IfEnoughPromisesReceivedAcceptMessageShouldBeSent) {
  Proposer p("foo", 2, "value");
  const auto prepare_msg = p.request_permission();
  p.process_promise(
      Message::PromiseMessage(prepare_msg.m_proposal_id, "bar", "value"));
  const auto response = p.process_promise(
      Message::PromiseMessage(prepare_msg.m_proposal_id, "baz", "value"));
  ASSERT_TRUE(response);
  ASSERT_EQ(response->m_proposal_id, prepare_msg.m_proposal_id);
  ASSERT_EQ(response->m_sender_id, "foo");
  ASSERT_EQ(response->m_value, "value");
}

TEST_F(PaxosTest, DontActOnPromisesForDifferentProposals) {
  Proposer p("foo", 2, "value");
  const auto prepare_msg = p.request_permission();
  p.process_promise(
      Message::PromiseMessage(prepare_msg.m_proposal_id, "bar", "foo"));
  const auto response = p.process_promise(Message::PromiseMessage(
      ProposalID("foo", prepare_msg.m_proposal_id.m_proposal_id + 1), "baz",
      "foo"));
  ASSERT_FALSE(response);
}

TEST_F(PaxosTest, CheckPromisesForHigherProposalID) {
  Proposer p("foo", 2, "value");
  const auto prepare_msg = p.request_permission();
  p.process_promise(Message::PromiseMessage(
      ProposalID("foo", prepare_msg.m_proposal_id.m_proposal_id + 1), "baz",
      "value"));
  const auto prepare_msg2 = p.request_permission();
  ASSERT_EQ(prepare_msg2.m_proposal_id.m_proposal_id,
            prepare_msg.m_proposal_id.m_proposal_id + 2);
}

TEST_F(PaxosTest, ProposerCheckPromisesWithHigherProposalID) {
  Proposer p("foo", 2, "value");
  const auto prepare_msg = p.request_permission();
  p.process_promise(Message::PromiseMessage(
      ProposalID("foo", prepare_msg.m_proposal_id.m_proposal_id + 1), "baz",
      "value2"));
  const auto prepare_msg2 = p.request_permission();
  ASSERT_EQ(prepare_msg2.m_value, "value2");
}

TEST_F(PaxosTest, ProposerIgnorsEmptyValueInPromiseMessage) {
  Proposer p("foo", 2, "value");
  const auto prepare_msg = p.request_permission();
  p.process_promise(Message::PromiseMessage(
      ProposalID("foo", prepare_msg.m_proposal_id.m_proposal_id + 1), "baz",
      ""));
  const auto prepare_msg2 = p.request_permission();
  ASSERT_EQ(prepare_msg2.m_value, "value");
}

TEST_F(PaxosTest, AcceptorRespondsWithPromiseToPrepareMessage) {
  Acceptor a("foo");
  const Message::PrepareMessage prepare_msg(ProposalID("bar", 2), "foo value");
  const auto response = a.process_prepare(prepare_msg);
  ASSERT_EQ(response->m_type, Message::Type::Promise);
  ASSERT_EQ(response->m_proposal_id, prepare_msg.m_proposal_id);
  ASSERT_EQ(response->m_sender_id, "foo");
}

TEST_F(PaxosTest, AcceptorRejectPrepareMsgsIfPromisedHigherProposalID) {
  Acceptor a("foo");
  const Message::PrepareMessage prepare_msg(ProposalID("bar", 2), "foo value");
  a.process_prepare(prepare_msg);
  const Message::PrepareMessage snd_prepare_msg(
      ProposalID("baz", prepare_msg.m_proposal_id.m_proposal_id - 1),
      "bar value");
  const auto response = a.process_prepare(snd_prepare_msg);
  ASSERT_EQ(response->m_type, Message::Type::NoAck);
}

TEST_F(PaxosTest, AcceptorSendsEmptyValueInPromiseBeforeAnyMessageIsAccepted) {
  Acceptor a("foo");
  const Message::PrepareMessage prepare_msg(ProposalID("bar", 2), "foo");
  const auto response = a.process_prepare(prepare_msg);
  const auto promise_msg = dynamic_cast<Message::PromiseMessage &>(*response);
  ASSERT_EQ(promise_msg.m_value, "");
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
