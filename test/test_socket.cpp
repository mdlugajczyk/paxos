#include "socket.h"
#include "gtest/gtest.h"
#include <future>

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

TEST_F(SocketTest, MultipleSenders) {
  const unsigned short port = 8077;
  const string rcv_host("localhost");
  Receiver rcv(port);
  Sender snd1(rcv_host, port);
  Sender snd2(rcv_host, port);

  snd1.send("foobarbaz");
  snd2.send("trololo");
  ASSERT_EQ(rcv.recv(), "foobarbaz");
  ASSERT_EQ(rcv.recv(), "trololo");
}

TEST_F(SocketTest, BlockUntilDataArrives) {
  const unsigned short port = 8077;
  const string rcv_host("localhost");
  Receiver rcv(port);
  auto future = std::async(std::launch::async, [&] { return rcv.recv(); });

  ASSERT_EQ(future.wait_for(std::chrono::seconds(1)),
            std::future_status::timeout);
  ;

  Sender snd(rcv_host, port);
  snd.send("foobarbaz");
  ASSERT_EQ(future.get(), "foobarbaz");
}

TEST_F(SocketTest, LazilyConnect) {
  const unsigned short port = 8077;
  const string rcv_host("localhost");
  Sender snd(rcv_host, port);
  ASSERT_EQ(snd.send("foobarbaz"), false);
  Receiver rcv(port);
  ASSERT_EQ(snd.send("foobarbaz"), true);
  ASSERT_EQ(rcv.recv(), "foobarbaz");
}

TEST_F(SocketTest, SendTwice) {
  const unsigned short port = 8077;
  const string rcv_host("localhost");
  Sender snd(rcv_host, port);
  Receiver rcv(port);
  ASSERT_EQ(snd.send("foobarbaz"), true);
  ASSERT_EQ(rcv.recv(), "foobarbaz");
  ASSERT_EQ(snd.send("trololo"), true);
  ASSERT_EQ(rcv.recv(), "trololo");
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
