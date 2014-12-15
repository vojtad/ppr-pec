cmake_minimum_required(VERSION 2.8.4)
project(ppr_pec)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp Move.cpp Game.cpp GameBoard.cpp)

if(BUILD_PARALLEL)
message("Building parallel.")

find_package(MPI REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MPI_COMPILE_FLAGS}")
set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} ${MPI_LINK_FLAGS}")

include_directories(${MPI_INCLUDE_PATH})

add_definitions(-DBUILD_PARALLEL)
set(SOURCE_FILES ${SOURCE_FILES} MyMPI.cpp ParallelGame.cpp)

add_executable(ppr_pec ${SOURCE_FILES})
target_link_libraries(ppr_pec ${MPI_LIBRARIES})
else()
message("Building sequential.")
set(SOURCE_FILES ${SOURCE_FILES} SequentialGame.cpp)

add_executable(ppr_pec ${SOURCE_FILES})
endif(BUILD_PARALLEL)
