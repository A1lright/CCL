cmake_minimum_required(VERSION 3.18)
project(CCL)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

find_package(LLVM REQUIRED CONFIG)
list(APPEND CMAKE_MODULE_PATH ${LLVM_CMAKE_DIR})

include(AddLLVM)
include_directories("${LLVM_BINARY_DIR}/include" "${LLVM_INCLUDE_DIR}")
add_definitions(${LLVM_DEFINITIONS})

# 添加头文件搜索路径
include_directories(/home/lin/CCl/include)

# 移除 -fno-exceptions 选项
string(REPLACE "-fno-exceptions" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

# 添加 -fexceptions 选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions")

# 移除重复的 -fexceptions 选项
string(REGEX REPLACE "-fexceptions( -fexceptions)+" "-fexceptions" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

# 修改这里：添加 TargetParser 组件
set(LLVM_LINK_COMPONENTS Support Core TargetParser)

# 添加可执行文件
add_llvm_executable(${PROJECT_NAME} 
    ./src/main.cpp  
    ./src/lexer.cpp  
    ./src/parser.cpp   
    ./src/codeGenerator.cpp
    ./src/symbolTable.cpp
    ./src/SymbolTypeBuilder.cpp
)

# 为目标可执行文件明确指定编译选项
target_compile_options(${PROJECT_NAME} PRIVATE -fexceptions)

# 再次确保移除 -fno-exceptions 选项
get_target_property(current_flags ${PROJECT_NAME} COMPILE_FLAGS)
if(current_flags)
    string(REPLACE "-fno-exceptions" "" new_flags "${current_flags}")
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "${new_flags}")
else()
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-fexceptions")
endif()

# 链接 LLVM 库
llvm_config(${PROJECT_NAME} ${LLVM_LINK_COMPONENTS})
