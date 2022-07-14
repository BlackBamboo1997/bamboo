cmake_minimum_required(VERSION 3.0)

project(SUDOKU_SERVER)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -pthread")

include_directories(
    ${CMAKE_SOURCE_DIR}/base/include
    ${CMAKE_SOURCE_DIR}/net/include
    ${CMAKE_SOURCE_DIR}/net/include/poller
)

add_executable(
 Sudoku_server_test_exe
 net/test/Sudoku_server_test.cpp 
#  echo_servet_test_exe
#  net/test/echo_servet_test.cpp
 base/src/CurrentThread.cpp
 base/src/Logging.cpp 
 base/src/LogStream.cpp 
 base/src/Thread.cpp  
 base/src/ThreadPool.cpp 
 base/src/Timestamp.cpp 
 base/src/FileUtil.cpp 
 base/src/LogFile.cpp 
 base/src/AsyncLogging.cpp
 net/src/poller/Poller.cpp
 net/src/poller/PollPoller.cpp
 net/src/poller/EPollPoller.cpp
 net/src/Acceptor.cpp
 net/src/Buffer.cpp
 net/src/Channel.cpp
 net/src/EventLoop.cpp
 net/src/EventLoopThread.cpp
 net/src/EventLoopThreadPool.cpp
 net/src/InetAddress.cpp
 net/src/Socket.cpp
 net/src/SocketsOps.cpp
 net/src/TcpConnection.cpp
 net/src/TcpServer.cpp
)