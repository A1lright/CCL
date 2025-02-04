add_executable(test_CLexer test_CLexer.cpp)
target_include_directories(test_CLexer PRIVATE ../include)
target_link_libraries(test_CLexer PRIVATE CLexer)