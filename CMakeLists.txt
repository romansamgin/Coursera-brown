cmake_minimum_required(VERSION 3.10.2)

project(TransportRouterE)

set(SOURCES main.cpp database.cpp geometry.cpp json.cpp)
set(HEADERS database.h geometry.h json.h graph.h router.h)
set(CMAKE_CXX_STANDARD 17)

add_executable(TransportRouterE ${SOURCES} ${HEADERS})

set_target_properties(
	TransportRouterE PROPERTIES
	CXX_STANDART 17
	CXX_STANDART_REQUIRED ON
)
