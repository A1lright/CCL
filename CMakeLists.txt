cmake_minimum_required(VERSION 3.10)
project(CCL)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CONFIGURE_DEPENDS "${SOURCE_FILES}")
# 添加头文件搜索路径
include_directories(/home/lin/CCl/include)

# 添加可执行文件
add_executable(main /home/lin/CCl/main.cpp /home/lin/CCl/src/lexer.cpp)
