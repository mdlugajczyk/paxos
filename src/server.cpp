#include "driver.h"
#include <cstdlib>
#include <iostream>

using namespace std;

unsigned short to_short(char *str) {
  return static_cast<unsigned short>(std::strtoul(str, nullptr, 10));
}

int main(int argc, char **argv) {
  if (argc < 8) {
    cout << argv[0] << " node_name port data_dir node1 port1 node2 port2 ..."
         << endl;
    return 1;
  }

  const unsigned short port = to_short(argv[2]);

  vector<Host> hosts;
  for (int i = 4; i < argc - 1; i += 2) {
    hosts.emplace_back(argv[i], to_short(argv[i + 1]));
  }

  cout << "enter value" << endl;
  string value;
  getline(cin, value);

  std::cout << value.empty() << endl;

  PaxosDriver driver(hosts, port, argv[1], argv[3], value);

  driver.run();

  return 0;
}
