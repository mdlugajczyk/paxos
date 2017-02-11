#include "socket.h"
#include "gtest/gtest.h"

using namespace std;

class SocketTest : public ::testing::Test {};

TEST_F(SocketTest, SendReceive) {
  const unsigned short port = 8077;
  const string rcv_host("localhost");
  Receiver rcv(port);
  Sender snd(rcv_host, port);

  snd.send("foobarbaz");
  ASSERT_EQ(rcv.recv(), "foobarbaz");
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
