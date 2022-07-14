# cmake_minimum_required(VERSION 3.0)

# project(ASYNCLOGGING)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -pthread")

# include_directories(${CMAKE_SOURCE_DIR}/include)

# # add_executable(Logging_test_exe Logging_test.cpp src/CurrentThread.cpp src/Logging.cpp src/LogStream.cpp src/Thread.cpp src/ThreadPool.cpp src/Timestamp.cpp)
# # add_executable(LogFile_test_exe test/LogFile_test.cpp src/CurrentThread.cpp src/Logging.cpp src/LogStream.cpp src/Thread.cpp src/ThreadPool.cpp src/Timestamp.cpp src/FileUtil.cpp src/LogFile.cpp)
# add_executable(AsyncLogging_test_exe test/AsyncLogging_test.cpp src/CurrentThread.cpp src/Logging.cpp src/LogStream.cpp src/Thread.cpp src/ThreadPool.cpp src/Timestamp.cpp src/FileUtil.cpp src/LogFile.cpp src/AsyncLogging.cpp)

cmake_minimum_required(VERSION 3.0)

project(EVENTLOOP)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -pthread")

include_directories(${CMAKE_SOURCE_DIR}/include bamboo/base/include)

add_executable(
 AsyncLogging_test_exe
 test/AsyncLogging_test.cpp 
 src/CurrentThread.cpp
 src/Logging.cpp 
 src/LogStream.cpp 
 src/Thread.cpp  
 src/ThreadPool.cpp 
 src/Timestamp.cpp 
 src/FileUtil.cpp 
 src/LogFile.cpp 
 src/AsyncLogging.cpp
)