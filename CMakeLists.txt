cmake_minimum_required(VERSION 3.18)
project(CCL)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

# 🔥 关键修改 1：确保 CMake 能找到 LLVM
if(NOT DEFINED LLVM_DIR)
    set(LLVM_DIR "/home/lin/llvm_project_1706/llvm_install_dir/lib/cmake/llvm")  # 设置 LLVM_DIR
endif()

# 🔥 关键修改 2：找到 LLVM 并仅使用 MIPS 组件
find_package(LLVM REQUIRED CONFIG)

list(APPEND CMAKE_MODULE_PATH ${LLVM_CMAKE_DIR})

include(AddLLVM)
include_directories("${LLVM_BINARY_DIR}/include" "${LLVM_INCLUDE_DIR}")
# add_definitions(${LLVM_DEFINITIONS})

# 添加头文件搜索路径
include_directories(/home/lin/CCl/include)

# 🔥 关键修改 3：移除 X86，添加 MIPS 组件
set(LLVM_LINK_COMPONENTS 
    Support 
    Core 
    TargetParser
    MC          # 机器码层组件
    # 🔥 MIPS 组件（仍然保留）
    MipsCodeGen
    MipsAsmParser
    MipsDesc
    MipsInfo
    # 🔥 X86 组件（新增）
    X86CodeGen
    X86AsmParser
    X86Desc
    X86Info
)

# 添加可执行文件
add_llvm_executable(${PROJECT_NAME} 
    ./src/main.cpp  
    ./src/lexer.cpp  
    ./src/parser.cpp   
    ./src/symbolTable.cpp
    ./src/SemanticAnalyzer.cpp
    ./src/evalConstant.cpp
    ./src/codeGenerator.cpp
)

# 为目标可执行文件明确指定编译选项
target_compile_options(${PROJECT_NAME} PRIVATE -frtti -fexceptions)

# 🔥 关键修改 4：正确链接 LLVM 组件
llvm_config(${PROJECT_NAME} ${LLVM_LINK_COMPONENTS})