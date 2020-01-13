find_package(RapidJSON)
add_executable(mesh-json mesh-json.cpp)
target_link_libraries(mesh-json pugixml)
