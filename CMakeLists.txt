cmake_minimum_required(VERSION 3.10.2)

set(SOURCES main.cpp database.cpp geometry.cpp json.cpp)
set(HEADERS database.h geometry.h json.h graph.h router.h)
set(CMAKE_CXX_STANDARD 17)

add_executable(TransportRouterE ${SOURCES} ${HEADERS})
