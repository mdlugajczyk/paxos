#ifndef __PERSISTENCE_H_
#define __PERSISTENCE_H_

#include <string>
#include <vector>

namespace Paxos {
namespace Persistence {

class Serializer {
public:
  std::string str() const;
  void serialize(int val);
  void serialize(const std::string &str);

private:
  void serialize(const char *const val, int val_size);
  std::vector<char> m_data;
};

class Deserializer {
public:
  Deserializer(const std::string &serialized_value);
  template <typename T> T deserialize();

private:
  int m_index;
  const std::string m_data;
};

template <> std::string Deserializer::deserialize();
template <> int Deserializer::deserialize();
}
}

#endif
