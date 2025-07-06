# CMakeList.txt: blog-encrypt 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

# 如果支持，请为 MSVC 编译器启用热重载。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("reimuEncrypt")

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# cryptopp-cmake忽略测试
set(CRYPTOPP_BUILD_TESTING OFF CACHE BOOL "Build testing" FORCE)
set(CRYPTOPP_BUILD_DOCUMENTATION OFF CACHE BOOL "Build documentation" FORCE)

# 添加 Crypto++ CMake 项目
set(CryptoCMake_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/cryptopp-cmake")
add_subdirectory("${CryptoCMake_DIR}")

# 添加Lexbor子项目
set(LEXBOR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/lexbor")
add_subdirectory("${LEXBOR_DIR}")

# 添加 cpp-base64 库
set(CPP_BASE64_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/cpp-base64")
set(CPP_BASE64_SOURCES
    ${CPP_BASE64_DIR}/base64.cpp
)

# 添加 nlohmann-json 库
set(NlohmannJson_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/json")
add_subdirectory("${NlohmannJson_DIR}")

# 添加包含目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CPP_BASE64_DIR}
    ${LEXBOR_DIR}/source
    ${LEXBOR_DIR}/source/lexbor/html
    ${LEXBOR_DIR}/source/lexbor/css
    ${LEXBOR_DIR}/source/lexbor/selectors
)

# 设置项目源文件
set(PROJECT_SOURCE_FILES
    aceEncrypt.h
    aceEncrypt.cpp  # 包含加密逻辑
    prase.h
    prase.cpp  # 包含解析html逻辑
    main.cpp  # 主程序入口
    ${CPP_BASE64_SOURCES}
)

# 创建可执行文件
add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE 
    cryptopp::cryptopp
    lexbor_static
    nlohmann_json::nlohmann_json
)

# TODO: 如有需要，请添加测试并安装目标。