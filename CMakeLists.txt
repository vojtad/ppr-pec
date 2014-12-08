cmake_minimum_required(VERSION 2.8.4)
project(ppr_pec)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp Move.cpp Game.cpp GameBoard.cpp)

#add_definitions(-DBUILD_PARALLEL)
#set(SOURCE_FILES ${SOURCE_FILES} MPI.cpp ParallelGame.cpp)

set(SOURCE_FILES ${SOURCE_FILES} SequentialGame.cpp)

add_executable(ppr_pec ${SOURCE_FILES})