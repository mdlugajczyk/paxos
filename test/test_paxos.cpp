#include "fake_state_persister.h"
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
  const auto msg = p.request_permission("value");
  ASSERT_EQ(msg->m_proposal_id.m_node_id, "foo");
  ASSERT_EQ(msg->m_proposal_id.m_proposal_id, 1);
  ASSERT_EQ(msg->m_value, "value");
}

TEST_F(PaxosTest, ProposerNewPermissionRequestNewID) {
  Proposer p("foo", 2);
  const auto first_msg = p.request_permission("value");
  const auto second_msg = p.request_permission("value");
  ASSERT_EQ(first_msg->m_proposal_id.m_proposal_id, 1);
  ASSERT_EQ(second_msg->m_proposal_id.m_proposal_id, 2);
}

TEST_F(PaxosTest, ProposerCantBeCreatedWithQuorumBelow2) {
  ASSERT_THROW(Proposer("foo", 1), QuorumTooSmallException);
}

TEST_F(PaxosTest, ReceiveLessThanQuorumRejectedMessagesAfterProposal) {
  Proposer p("foo", 2);
  const auto permission_msg = p.request_permission("value");
  const Message::NoAck noack("bar", permission_msg->m_proposal_id,
                             ProposalID("baz", 2), "");
  const auto nack_response = p.process_noack(noack);
  ASSERT_FALSE(nack_response);
}

TEST_F(PaxosTest, IfQuorumRejectedProposalShouldSendNew) {
  Proposer p("foo", 2);
  const auto permission_msg = p.request_permission("value");
  const ProposalID accepted_proposal("baz", 3);
  const Message::NoAck noack1("bar", permission_msg->m_proposal_id,
                              accepted_proposal, "");
  p.process_noack(noack1);
  const Message::NoAck noack2("baz", permission_msg->m_proposal_id,
                              accepted_proposal, "");
  const auto noack_response = p.process_noack(noack2);
  ASSERT_TRUE(noack_response);
  ASSERT_EQ(noack_response->m_sender_id, "foo");
  ASSERT_EQ(noack_response->m_proposal_id.m_node_id, "foo");
  ASSERT_EQ(noack_response->m_proposal_id.m_proposal_id, 4);
  ASSERT_EQ(noack_response->m_value, "value");
}

TEST_F(PaxosTest, IgnoreQuorumRejectingSomeoneElseProposal) {
  Proposer p("foo", 2);
  const auto permission_msg = p.request_permission("value");
  const ProposalID accepted_proposal("baz", 3);
  const ProposalID rejected_proposal("bar", 4);
  const Message::NoAck noack1("bar", rejected_proposal, accepted_proposal, "");
  p.process_noack(noack1);
  const Message::NoAck noack2("baz", rejected_proposal, accepted_proposal, "");
  const auto noack_response = p.process_noack(noack2);
  ASSERT_FALSE(noack_response);
}

TEST_F(PaxosTest, CheckNoAcksForHighestProposalID) {
  Proposer p("foo", 2);
  p.request_permission("value");
  const ProposalID accepted_proposal("bar", 4);
  const Message::NoAck noack("bar", ProposalID("baz", 3), accepted_proposal,
                             "");
  p.process_noack(noack);
  const auto permission_request = p.request_permission("value");
  ASSERT_EQ(permission_request->m_proposal_id,
            ProposalID("foo", accepted_proposal.m_proposal_id + 1));
}

TEST_F(PaxosTest, DontActOnPromisesUntilQuorumIsReached) {
  Proposer p("foo", 2);
  const auto prepare_msg = p.request_permission("value");
  const auto response = p.process_promise(
      Message::PromiseMessage(prepare_msg->m_proposal_id, "bar", ""));
  ASSERT_FALSE(response);
}

TEST_F(PaxosTest, IfEnoughPromisesReceivedAcceptMessageShouldBeSent) {
  Proposer p("foo", 2);
  const auto prepare_msg = p.request_permission("value");
  p.process_promise(
      Message::PromiseMessage(prepare_msg->m_proposal_id, "bar", "value"));
  const auto response = p.process_promise(
      Message::PromiseMessage(prepare_msg->m_proposal_id, "baz", "value"));
  ASSERT_TRUE(response);
  ASSERT_EQ(response->m_proposal_id, prepare_msg->m_proposal_id);
  ASSERT_EQ(response->m_sender_id, "foo");
  ASSERT_EQ(response->m_value, "value");
}

TEST_F(PaxosTest, DontActOnPromisesForDifferentProposals) {
  Proposer p("foo", 2);
  const auto prepare_msg = p.request_permission("value");
  p.process_promise(
      Message::PromiseMessage(prepare_msg->m_proposal_id, "bar", "foo"));
  const auto response = p.process_promise(Message::PromiseMessage(
      ProposalID("foo", prepare_msg->m_proposal_id.m_proposal_id + 1), "baz",
      "foo"));
  ASSERT_FALSE(response);
}

TEST_F(PaxosTest, CheckPromisesForHigherProposalID) {
  Proposer p("foo", 2);
  const auto prepare_msg = p.request_permission("value");
  p.process_promise(Message::PromiseMessage(
      ProposalID("foo", prepare_msg->m_proposal_id.m_proposal_id + 1), "baz",
      "new value"));
  const auto prepare_msg2 = p.request_permission("value");
  ASSERT_EQ(prepare_msg2->m_proposal_id.m_proposal_id,
            prepare_msg->m_proposal_id.m_proposal_id + 2);

  ASSERT_EQ(prepare_msg2->m_value, "new value");
}

TEST_F(PaxosTest, ProposerCheckPromisesWithHigherProposalID) {
  Proposer p("foo", 2);
  const auto prepare_msg = p.request_permission("value");
  p.process_promise(Message::PromiseMessage(
      ProposalID("foo", prepare_msg->m_proposal_id.m_proposal_id + 1), "baz",
      "value2"));
  const auto prepare_msg2 = p.request_permission("value");
  ASSERT_EQ(prepare_msg2->m_value, "value2");
}

TEST_F(PaxosTest, ProposerIgnorsEmptyValueInPromiseMessage) {
  Proposer p("foo", 2);
  const auto prepare_msg = p.request_permission("value");
  p.process_promise(Message::PromiseMessage(
      ProposalID("foo", prepare_msg->m_proposal_id.m_proposal_id + 1), "baz",
      ""));
  const auto prepare_msg2 = p.request_permission("value");
  ASSERT_EQ(prepare_msg2->m_value, "value");
}

TEST_F(PaxosTest, ProposerChecksValueFromThePromiseMessage) {
  Proposer p("foo", 2);
  const auto prepare_msg = p.request_permission("value");
  p.process_promise(Message::PromiseMessage(
      ProposalID("foo", prepare_msg->m_proposal_id.m_proposal_id), "baz",
      "different value"));
  const auto prepare_msg2 = p.request_permission("value");
  ASSERT_EQ(prepare_msg2->m_value, "different value");
}

TEST_F(PaxosTest, AcceptorRespondsWithPromiseToPrepareMessage) {
  Acceptor a("foo", std::make_shared<FakeStatePersister>(
                        Paxos::State("", ProposalID())));
  const Message::PrepareMessage prepare_msg(ProposalID("bar", 2), "foo value");
  const auto response = a.process_prepare(prepare_msg);
  ASSERT_EQ(response->m_type, Message::Type::Promise);
  const auto promise_msg = dynamic_cast<Message::PromiseMessage &>(*response);
  ASSERT_EQ(promise_msg.m_proposal_id, prepare_msg.m_proposal_id);
  ASSERT_EQ(promise_msg.m_sender_id, "foo");
}

TEST_F(PaxosTest, AcceptorRejectPrepareMsgsIfPromisedHigherProposalID) {
  Acceptor a("foo", std::make_shared<FakeStatePersister>(
                        Paxos::State("", ProposalID())));
  const Message::PrepareMessage prepare_msg(ProposalID("bar", 2), "foo value");
  a.process_prepare(prepare_msg);
  const Message::PrepareMessage snd_prepare_msg(
      ProposalID("baz", prepare_msg.m_proposal_id.m_proposal_id - 1),
      "bar value");
  const auto response = a.process_prepare(snd_prepare_msg);
  ASSERT_EQ(response->m_type, Message::Type::NoAck);
}

TEST_F(PaxosTest, AcceptorSendsEmptyValueInNoValueHasBeenAcceptedYet) {
  Acceptor a("foo", std::make_shared<FakeStatePersister>(
                        Paxos::State("", ProposalID())));
  const Message::PrepareMessage prepare_msg(ProposalID("bar", 2), "foo value");
  a.process_prepare(prepare_msg);
  const auto response = a.process_prepare(
      Message::PrepareMessage(ProposalID("asdf", 1), "new-value"));
  ASSERT_EQ(response->m_type, Message::Type::NoAck);
  const auto noack_msg = dynamic_cast<Message::NoAck &>(*response);
  ASSERT_EQ(noack_msg.m_accepted_value, "");
}

TEST_F(PaxosTest, NoAckShouldIncludeAcceptedValue) {
  Acceptor a("foo", std::make_shared<FakeStatePersister>(
                        Paxos::State("", ProposalID())));
  const Message::PrepareMessage prepare_msg(ProposalID("bar", 2), "foo value");
  a.process_prepare(prepare_msg);
  a.process_accept(Message::AcceptMessage(ProposalID("bar", 2), "foo value"));
  const auto response = a.process_prepare(
      Message::PrepareMessage(ProposalID("asdf", 1), "new-value"));
  ASSERT_EQ(response->m_type, Message::Type::NoAck);
  const auto noack_msg = dynamic_cast<Message::NoAck &>(*response);
  ASSERT_EQ(noack_msg.m_accepted_value, "foo value");
}

TEST_F(PaxosTest, NoAckFromAcceptRequestShouldIncludeAcceptedValue) {
  Acceptor a("foo", std::make_shared<FakeStatePersister>(
                        Paxos::State("", ProposalID())));
  const Message::PrepareMessage prepare_msg(ProposalID("bar", 2), "foo value");
  a.process_prepare(prepare_msg);
  a.process_accept(Message::AcceptMessage(ProposalID("bar", 2), "foo value"));
  const auto response = a.process_accept(Message::AcceptMessage(
      Message::AcceptMessage(ProposalID("asdf", 1), "new-value")));
  ASSERT_EQ(response->m_type, Message::Type::NoAck);
  const auto noack_msg = dynamic_cast<Message::NoAck &>(*response);
  ASSERT_EQ(noack_msg.m_accepted_value, "foo value");
}

TEST_F(PaxosTest, AcceptorSendsEmptyValueInPromiseBeforeAnyMessageIsAccepted) {
  Acceptor a("foo", std::make_shared<FakeStatePersister>(
                        Paxos::State("", ProposalID())));
  const Message::PrepareMessage prepare_msg(ProposalID("bar", 2), "foo");
  const auto response = a.process_prepare(prepare_msg);
  const auto promise_msg = dynamic_cast<Message::PromiseMessage &>(*response);
  ASSERT_EQ(promise_msg.m_value, "");
}

TEST_F(PaxosTest, AcceptorShouldAcceptProposalItJustPromisedToAccept) {
  Acceptor a("foo", std::make_shared<FakeStatePersister>(
                        Paxos::State("", ProposalID())));
  const ProposalID proposal_id("bar", 2);
  const Message::PrepareMessage prepare_msg(proposal_id, "value");
  const auto response = a.process_prepare(prepare_msg);
  const auto accepted =
      a.process_accept(Message::AcceptMessage(proposal_id, "value"));
  ASSERT_EQ(accepted->m_type, Message::Type::Accepted);
}

TEST_F(PaxosTest, AcceptorShouldntAcceptProposalWithLowerIDThanPromised) {
  Acceptor a("foo", std::make_shared<FakeStatePersister>(
                        Paxos::State("", ProposalID())));
  const ProposalID proposal_id("bar", 2);
  const Message::PrepareMessage prepare_msg(proposal_id, "value");
  const auto response = a.process_prepare(prepare_msg);
  const auto accepted =
      a.process_accept(Message::AcceptMessage(ProposalID("bar", 1), "value"));
  ASSERT_EQ(accepted->m_type, Message::Type::NoAck);
}

TEST_F(PaxosTest, DontAcceptProposalsWithLowerIDThanLastAcceptedProposal) {
  Acceptor a("foo", std::make_shared<FakeStatePersister>(
                        Paxos::State("", ProposalID())));
  const ProposalID proposal_id("bar", 1);
  const Message::PrepareMessage prepare_msg(proposal_id, "value");
  const auto response = a.process_prepare(prepare_msg);
  a.process_accept(Message::AcceptMessage(ProposalID("bar", 3), "value"));
  ASSERT_EQ(
      a.process_accept(Message::AcceptMessage(ProposalID("bar", 2), "value"))
          ->m_type,
      Message::Type::NoAck);
}

TEST_F(PaxosTest, AcceptedMessageHasTheCorrectValue) {
  Acceptor a("foo", std::make_shared<FakeStatePersister>(
                        Paxos::State("", ProposalID())));
  const ProposalID proposal_id("bar", 1);
  const auto response =
      a.process_accept(Message::AcceptMessage(ProposalID("bar", 3), "value"));
  const auto accepted = dynamic_cast<Message::AcceptedMessage &>(*response);
  ASSERT_EQ(accepted.m_value, "value");
}

TEST_F(PaxosTest, PromiseShouldIncludeAValueIfAlreadyAccepted) {
  Acceptor a("foo", std::make_shared<FakeStatePersister>(
                        Paxos::State("", ProposalID())));
  const ProposalID proposal_id("bar", 1);
  a.process_accept(Message::AcceptMessage(ProposalID("bar", 2), "value"));
  const auto response =
      a.process_prepare(Message::PrepareMessage(ProposalID("bar", 3), "fnord"));
  const auto promise_msg = dynamic_cast<Message::PromiseMessage &>(*response);
  ASSERT_EQ(promise_msg.m_value, "value");
}

TEST_F(PaxosTest, LearnerDoesntDeclaresConsensusUntilQuorumAgressOnValue) {
  Learner learner("foo", 3);
  const ProposalID proposal_id("bar", 1);
  const auto response = learner.process_accepted(
      Message::AcceptedMessage(proposal_id, "bar", "value"));
  ASSERT_FALSE(response);
}

TEST_F(PaxosTest, ShouldNoticeWhenQuorumAcceptsProposal) {
  Learner learner("foo", 3);
  const ProposalID proposal_id("bar", 1);
  ASSERT_FALSE(learner.process_accepted(
      Message::AcceptedMessage(proposal_id, "bar", "value")));
  ASSERT_FALSE(learner.process_accepted(
      Message::AcceptedMessage(proposal_id, "foo", "value")));
  const auto response = learner.process_accepted(
      Message::AcceptedMessage(proposal_id, "baz", "value"));
  ASSERT_TRUE(response);
  ASSERT_EQ(response->m_type, Message::Type::ConsensusReached);
}

TEST_F(PaxosTest, LearnerShouldDifferentiateBetweenDifferentProposals) {
  Learner learner("foo", 3);
  const ProposalID proposal_id("bar", 1);
  const ProposalID proposal_id2("baz", 2);
  ASSERT_FALSE(learner.process_accepted(
      Message::AcceptedMessage(proposal_id, "bar", "value")));
  ASSERT_FALSE(learner.process_accepted(
      Message::AcceptedMessage(proposal_id, "foo", "value")));
  ASSERT_FALSE(learner.process_accepted(
      Message::AcceptedMessage(proposal_id2, "foo", "value")));
}

TEST_F(PaxosTest, HandleDuplicatedMessages) {
  Learner learner("foo", 3);
  const ProposalID proposal_id("bar", 1);
  ASSERT_FALSE(learner.process_accepted(
      Message::AcceptedMessage(proposal_id, "bar", "value")));
  ASSERT_FALSE(learner.process_accepted(
      Message::AcceptedMessage(proposal_id, "foo", "value")));
  ASSERT_FALSE(learner.process_accepted(
      Message::AcceptedMessage(proposal_id, "foo", "value")));
}

TEST_F(PaxosTest,
       IfNodeApprovesDifferentProposalOriginalApprovalShouldntBeCounted) {
  Learner learner("foo", 3);
  const ProposalID proposal_id("bar", 1);
  const ProposalID proposal_id2("baz", 2);
  ASSERT_FALSE(learner.process_accepted(
      Message::AcceptedMessage(proposal_id, "bar", "value")));
  ASSERT_FALSE(learner.process_accepted(
      Message::AcceptedMessage(proposal_id, "foo", "value")));
  ASSERT_FALSE(learner.process_accepted(
      Message::AcceptedMessage(proposal_id2, "bar", "value")));
  ASSERT_FALSE(learner.process_accepted(
      Message::AcceptedMessage(proposal_id, "fnord", "value")));
}

TEST_F(PaxosTest, AcceptorRestoresPreviousState) {
  std::shared_ptr<FakeStatePersister> sp = std::make_shared<FakeStatePersister>(
      Paxos::State("value", ProposalID("a node", 3)));
  Acceptor a("foo", sp);
  ASSERT_EQ(sp->m_restore_calls, 1);
  const auto response =
      a.process_prepare(Message::PrepareMessage(ProposalID("bar", 1), "fnord"));
  ASSERT_EQ(response->m_type, Message::Type::NoAck);
  const auto noack = dynamic_cast<Message::NoAck &>(*response);
  ASSERT_EQ(noack.m_accepted_proposal, sp->m_state.m_proposal);

  const auto response2 =
      a.process_prepare(Message::PrepareMessage(ProposalID("bar", 4), "fnord"));
  ASSERT_EQ(response2->m_type, Message::Type::Promise);
  const auto promise = dynamic_cast<Message::PromiseMessage &>(*response2);
  ASSERT_EQ(promise.m_value, sp->m_state.m_value);
}

TEST_F(PaxosTest, AcceptorSavesStatBeforeSendingPromise) {
  std::shared_ptr<FakeStatePersister> sp =
      std::make_shared<FakeStatePersister>(Paxos::State("", ProposalID()));
  Acceptor a("foo", sp);
  const ProposalID proposal("bar", 1);
  const auto response =
      a.process_prepare(Message::PrepareMessage(proposal, "fnord"));
  ASSERT_EQ(response->m_type, Message::Type::Promise);
  ASSERT_EQ(sp->m_persist_calls, 1);
  ASSERT_EQ(sp->m_state.m_proposal, proposal);
}

TEST_F(PaxosTest, AcceptorSavesStatBeforeAcceptingProposal) {
  std::shared_ptr<FakeStatePersister> sp =
      std::make_shared<FakeStatePersister>(Paxos::State("", ProposalID()));
  Acceptor a("foo", sp);
  const ProposalID proposal("bar", 1);
  const auto response =
      a.process_accept(Message::AcceptMessage(proposal, "fnord"));
  ASSERT_EQ(response->m_type, Message::Type::Accepted);
  ASSERT_EQ(sp->m_persist_calls, 1);
  ASSERT_EQ(sp->m_state.m_proposal, proposal);
  ASSERT_EQ(sp->m_state.m_value, "fnord");
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
