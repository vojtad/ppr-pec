cmake_minimum_required(VERSION 2.8.4)
project(ppr_pec)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp MPI.cpp Move.cpp Game.cpp GameBoard.cpp GameCommunication.cpp SequentialGame.cpp ParallelGame.cpp)
add_executable(ppr_pec ${SOURCE_FILES})