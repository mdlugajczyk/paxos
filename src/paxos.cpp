#include "paxos.h"
#include "state_persister.h"
#include <iostream>

using namespace Paxos;

QuorumTooSmallException::QuorumTooSmallException()
    : std::runtime_error("Quorum size can't be smaller than 2.") {}

std::shared_ptr<Message::PrepareMessage>
Proposer::request_permission(const std::string &value) {
  if (m_value.empty())
    m_value = value;
  const ProposalID id(m_node_id, m_highest_proposal.m_proposal_id + 1);
  m_highest_proposal = id;
  m_current_proposal = id;
  return std::make_shared<Message::PrepareMessage>(id, m_value);
}

std::shared_ptr<Message::PrepareMessage>
Proposer::process_noack(const Message::NoAck &noack) {
  if (m_highest_proposal < noack.m_accepted_proposal)
    m_highest_proposal = noack.m_accepted_proposal;
  if (m_current_proposal != noack.m_rejected_proposal)
    return {};
  m_noack_senders.push_back(noack.m_sender_id);
  if (m_noack_senders.size() >= m_quorum_size)
    return request_permission(m_value);
  return {};
}

std::shared_ptr<Message::AcceptMessage>
Proposer::process_promise(const Message::PromiseMessage &promise) {
  if (m_highest_proposal < promise.m_proposal_id) {
    m_highest_proposal = promise.m_proposal_id;
    if (!promise.m_value.empty())
      m_value = promise.m_value;
  }
  if (promise.m_proposal_id != m_current_proposal)
    return {};
  m_promise_senders.push_back(promise.m_sender_id);
  if (m_promise_senders.size() >= m_quorum_size)
    return std::make_shared<Message::AcceptMessage>(m_current_proposal,
                                                    m_value);
  return {};
}

Proposer::Proposer(const std::string &id, const int quorum_size)
    : m_node_id(id), m_quorum_size(quorum_size),
      m_highest_proposal(m_node_id, 0), m_current_proposal(m_node_id, 0) {
  if (m_quorum_size < 2) {
    throw QuorumTooSmallException();
  }
}

Acceptor::Acceptor(const std::string &id,
                   std::shared_ptr<StatePersister> persister)
    : m_state_persister(persister), m_node_id(id),
      m_highest_proposal(m_node_id, 0) {
  const State s = m_state_persister->restore();
  m_highest_proposal = s.m_proposal;
  m_value = s.m_value;
}

std::shared_ptr<Message::Message>
Acceptor::process_prepare(const Message::PrepareMessage &prepare) {
  if (m_highest_proposal < prepare.m_proposal_id) {
    m_highest_proposal = prepare.m_proposal_id;
    m_state_persister->persist(State(m_value, m_highest_proposal));
    return std::make_shared<Message::PromiseMessage>(prepare.m_proposal_id,
                                                     m_node_id, m_value);
  }

  return std::make_shared<Message::NoAck>(m_node_id, prepare.m_proposal_id,
                                          m_highest_proposal);
}

std::shared_ptr<Message::Message>
Acceptor::process_accept(const Message::AcceptMessage &accept) {
  if (accept.m_proposal_id < m_highest_proposal)
    return std::make_shared<Message::NoAck>(m_node_id, accept.m_proposal_id,
                                            m_highest_proposal);
  m_highest_proposal = accept.m_proposal_id;
  m_value = accept.m_value;
  m_state_persister->persist(State(m_value, m_highest_proposal));
  return std::make_shared<Message::AcceptedMessage>(accept.m_proposal_id,
                                                    m_node_id, m_value);
}

Learner::Learner(const std::string &id, const int quorum_size)
    : m_id(id), m_quorum_size(quorum_size) {}

std::shared_ptr<Message::ConsensusReached>
Learner::process_accepted(const Message::AcceptedMessage &msg) {
  const auto previous_proposal = m_acceptors.find(msg.m_sender_id);
  if (previous_proposal != m_acceptors.end())
    m_accepted_proposals[previous_proposal->second].erase(
        previous_proposal->first);

  m_accepted_proposals[msg.m_proposal_id].insert(msg.m_sender_id);
  m_acceptors.insert(std::make_pair(msg.m_sender_id, msg.m_proposal_id));

  if (m_accepted_proposals[msg.m_proposal_id].size() >= m_quorum_size) {
    return std::make_shared<Message::ConsensusReached>(m_id, msg.m_value);
  }
  return {};
}

static std::string
get_path_to_node_state_file(const std::string &data_directory,
                            const std::string &node_name) {
  return data_directory + "/" + node_name;
}

Node::Node(int quorum_size, const std::string &node_name,
           const std::string &data_directory)
    : m_proposer(node_name, quorum_size),
      m_acceptor(node_name,
                 std::make_shared<StatePersister>(
                     get_path_to_node_state_file(data_directory, node_name))),
      m_learner(node_name, quorum_size) {}

static void notify_if_consensus_reached(std::shared_ptr<Message::Message> msg) {
  if (!msg)
    return;
  auto const consensus_reached =
      dynamic_cast<Message::ConsensusReached &>(*msg.get());
  std::cout << "Consensus reached. Value: " << consensus_reached.m_value
            << " sender " << consensus_reached.m_sender_id << std::endl;
}

std::shared_ptr<Message::Message>
Node::process_message(std::shared_ptr<Message::Message> msg) {
  // This switch statement isn't particulary nice, but I think it's better
  // then the alternative. To avoid it, something equivalent to a double
  // dispatch mechanism would be required, and that brings quite a lot of
  // complexity. I don't think that's justified, particularly as the Type enum
  // is not going to change.
  switch (msg->m_type) {
  case Message::Type::Prepare:
    return m_acceptor.process_prepare(
        dynamic_cast<Message::PrepareMessage &>(*msg.get()));
  case Message::Type::Promise:
    return m_proposer.process_promise(
        dynamic_cast<Message::PromiseMessage &>(*msg.get()));
  case Message::Type::Accept: {
    const auto response = m_acceptor.process_accept(
        dynamic_cast<Message::AcceptMessage &>(*msg.get()));
    if (response->m_type == Message::Type::Accepted)
      notify_if_consensus_reached(m_learner.process_accepted(
          dynamic_cast<Message::AcceptedMessage &>(*response.get())));
    return response;
  }
  case Message::Type::Accepted:
    return m_learner.process_accepted(
        dynamic_cast<Message::AcceptedMessage &>(*msg.get()));
  case Message::Type::NoAck:
    return m_proposer.process_noack(dynamic_cast<Message::NoAck &>(*msg.get()));
  case Message::Type::ConsensusReached:
    notify_if_consensus_reached(msg);
  }
  return {};
}

std::shared_ptr<Message::Message>
Node::propose_value(const std::string &value) {
  return m_proposer.request_permission(value);
}
