set(CMAKE_BUILD_TYPE Debug)

add_executable(qsqlcipher-test main.cpp)
target_link_libraries(qsqlcipher-test Qt5::Sql)

#add_test(NAME qsqlcipher-test COMMAND qsqlcipher-test)
