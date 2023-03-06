cmake_minimum_required(VERSION 3.20)
project(Thusly LANGUAGES C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
# set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)

add_subdirectory(src)

# add_executable(cthusly src/main.c)

add_executable(cthusly
	src/main.c
	src/common.h
	src/debug.h
	src/debug.c
	src/memory.h
	src/memory.c
	src/program.h
	src/program.c
	src/thusly_value.h
	src/thusly_value.c
)

target_include_directories(cthusly PUBLIC src)

# target_link_libraries(cthusly PUBLIC ..)
