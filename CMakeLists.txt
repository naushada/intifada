cmake_minimum_required(VERSION 3.16.3)
project(intifada)

#Adding Complier flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -g -Wall -Wextra")

include_directories(inc)

#However, the file(GLOB...) allows for wildcard additions:
file(GLOB SOURCES "src/*.cpp")

add_executable(intifada ${SOURCES})

target_link_libraries(intifada pthread ssl crypto)

add_subdirectory(test)

install(TARGETS intifada
	    CONFIGURATIONS Release
        RUNTIME DESTINATION /usr/local/bin
)