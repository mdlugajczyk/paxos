#include "persistence.h"
#include <cstring>
#include <stdexcept>

using namespace std;
using namespace Paxos::Persistence;

string Serializer::str() const { return string(&m_data[0], m_data.size()); };

void Serializer::serialize(int val) {
  serialize(reinterpret_cast<const char *>(&val), sizeof(val));
}

void Serializer::serialize(const string &str) {
  serialize(str.size());
  serialize(str.c_str(), str.size());
}

void Serializer::serialize(const char *const val, int val_size) {
  const int current_size = m_data.size();
  m_data.resize(current_size + val_size);
  memcpy(&m_data[current_size], val, val_size);
}

Deserializer::Deserializer(const string &serialized_value)
    : m_index(0), m_data(serialized_value) {}

template <> int Deserializer::deserialize() {
  if (m_index + sizeof(int) > m_data.size())
    throw std::runtime_error("Failed to deserialize int.");
  int res = 0;
  memcpy(&res, m_data.c_str() + m_index, sizeof(res));
  m_index += sizeof(res);
  return res;
}

template <> string Deserializer::deserialize() {
  const int length = deserialize<int>();
  if (m_index + length > m_data.size())
    throw std::runtime_error("Failed to deserialize string.");
  char buff[length];
  memcpy(buff, m_data.c_str() + m_index, length);
  m_index += length;
  return string(buff, length);
}
