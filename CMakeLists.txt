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

# 在CMakeLists.txt中启用LTO
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.9")
  # 直接设置链接时间优化，而不使用复杂的检查
  option(CMAKE_INTERPROCEDURAL_OPTIMIZATION "启用链接时间优化" ON)
  message(STATUS "已设置链接时间优化选项(IPO/LTO)")
else()
  message(STATUS "CMake 版本太低，无法设置 IPO/LTO")
endif()

# 对于 MSVC 编译器，添加特定的链接时间优化标志
if(MSVC)
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GL")
  set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CMAKE_STATIC_LINKER_FLAGS_RELEASE} /LTCG")
  set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /LTCG")
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")
  message(STATUS "为 MSVC 设置了 /GL 和 /LTCG 优化标志")
endif()

# 添加函数级优化标志
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections")
  if(UNIX AND NOT APPLE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")
  endif()
elseif(MSVC)
  # MSVC有自己的优化选项
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /OPT:REF /OPT:ICF")
endif()

# 优化二进制大小
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Os")
elseif(MSVC)
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O1")
endif()


# 添加 Crypto++ CMake 项目
set(CryptoCMake_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/cryptopp-cmake")

# cryptopp-cmake忽略测试
set(CRYPTOPP_BUILD_TESTING OFF CACHE BOOL "Build testing" FORCE)
set(CRYPTOPP_BUILD_DOCUMENTATION OFF CACHE BOOL "Build documentation" FORCE)
add_subdirectory("${CryptoCMake_DIR}")

# 添加Lexbor子项目
set(LEXBOR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/lexbor")

# 优化 Lexbor 子项目，只包含 HTML 解析相关功能
set(LEXBOR_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(LEXBOR_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(LEXBOR_BUILD_SEPARATELY OFF CACHE BOOL "" FORCE)
set(LEXBOR_BUILD_SHARED OFF CACHE BOOL "" FORCE)
# 只启用必要的模块
set(LEXBOR_BUILD_HTML ON CACHE BOOL "" FORCE)
set(LEXBOR_BUILD_DOM ON CACHE BOOL "" FORCE)
set(LEXBOR_BUILD_CSS ON CACHE BOOL "" FORCE)
set(LEXBOR_BUILD_SELECTORS ON CACHE BOOL "" FORCE)
# 禁用不需要的模块
set(LEXBOR_BUILD_ENCODING OFF CACHE BOOL "" FORCE)
set(LEXBOR_BUILD_HTTP OFF CACHE BOOL "" FORCE)
set(LEXBOR_BUILD_NS OFF CACHE BOOL "" FORCE)
set(LEXBOR_BUILD_UNICODE OFF CACHE BOOL "" FORCE)

add_subdirectory("${LEXBOR_DIR}")

# # 添加 cpp-base64 库 - 直接包含源文件
# set(CPP_BASE64_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/cpp-base64")
# set(CPP_BASE64_SOURCES
#     ${CPP_BASE64_DIR}/base64.cpp
# )

# 添加 nlohmann-json 库
set(NlohmannJson_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/json")
add_subdirectory("${NlohmannJson_DIR}")

# 添加包含目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    # ${CPP_BASE64_DIR}
    ${LEXBOR_DIR}/source
    ${LEXBOR_DIR}/source/lexbor/html
    ${LEXBOR_DIR}/source/lexbor/css
    ${LEXBOR_DIR}/source/lexbor/selectors
)

# 设置项目源文件
set(PROJECT_SOURCE_FILES
    aceEncrypt.h
    aceEncrypt.cpp  # 包含加密逻辑
    praseHtml.h
    praseHtml.cpp  # 包含解析html逻辑
    encryptConfig.h
    encryptConfig.cpp  # 加载加密配置json
    tool.h
    tool.cpp  # 包含工具函数和日志记录逻辑
    main.cpp  # 主程序入口
)

# 创建可执行文件
add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE 
    cryptopp::cryptopp
    lexbor_static
    nlohmann_json::nlohmann_json
)

# TODO: 如有需要，请添加测试并安装目标。