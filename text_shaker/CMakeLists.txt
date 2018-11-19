cmake_minimum_required(VERSION 3.12.0)

project(TextShaker)

find_package(Boost REQUIRED COMPONENTS program_options)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -mtune=native")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra -Werror")

include_directories(
	${Boost_INCLUDE_DIRS}
)

link_directories(
	${Boost_LIBRARY_DIRS}
)

set(src_list
	text_shaker.cpp
)

add_executable(text_shaker ${src_list})
target_link_libraries(text_shaker
	${Boost_LIBRARIES}
)
