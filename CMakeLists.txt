cmake_minimum_required(VERSION 3.10)
find_package(RapidJSON)
find_package(Boost 1.36.0 COMPONENTS iostreams)

include_directories(${Boost_INCLUDE_DIRS})
add_executable(mesh-json mesh-json.cpp)
target_link_libraries(mesh-json pugixml ${Boost_LIBRARIES})
