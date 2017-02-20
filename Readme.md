This is a simple, educational implementation of the Paxos algorithm.
The core of the algorithm is implemented in src/paxos.cpp, it has
a decent test coverage and should be mostly correct. However, the surrounding
code, particularly the network code isn't very robust. It was mostly written
just to test the logic in paxos.cpp. Similarly, serialization and persistance
have been implemented as simply as possible.