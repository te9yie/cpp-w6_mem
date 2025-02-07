cmake_minimum_required(VERSION 3.16)
project(w6_mem)

option(W6_MEM_ENABLE_TESTS "Enable tests" OFF)
option(W6_MEM_ENABLE_STATIC_ANALYSIS "Enable static analysis with clang-tidy" OFF)

include(FetchContent)

# t9_mem
add_library(${PROJECT_NAME} INTERFACE)
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_17)
target_compile_options(${PROJECT_NAME} INTERFACE
  $<$<CXX_COMPILER_ID:MSVC>:/utf-8 /W4 /WX /EHa- /GR->
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror -fno-exceptions -fno-rtti>
)
target_compile_definitions(${PROJECT_NAME} INTERFACE
  $<$<CXX_COMPILER_ID:MSVC>:_HAS_EXCEPTIONS=0>
)
target_include_directories(${PROJECT_NAME} INTERFACE include)

# 静的解析の設定
if(W6_MEM_ENABLE_STATIC_ANALYSIS)
    find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
    if(CLANG_TIDY_EXE)
        set(CLANG_TIDY_COMMAND 
            ${CLANG_TIDY_EXE}
            -p=${CMAKE_BINARY_DIR}
            --config-file=${CMAKE_SOURCE_DIR}/.clang-tidy
        )
        set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY_COMMAND})
        message(STATUS "Static analysis enabled with clang-tidy: ${CLANG_TIDY_EXE}")
    else()
        message(WARNING "clang-tidy not found. Static analysis will be disabled.")
    endif()
endif()

# tests
if (W6_MEM_ENABLE_TESTS)
    enable_testing()

    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.15.x
        GIT_SHALLOW TRUE
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(BUILD_GMOCK OFF)
    set(INSTALL_GTEST OFF)
    FetchContent_MakeAvailable(googletest)

    add_executable(${PROJECT_NAME}_test
        tests/allocator_test.cpp
    )
    target_link_libraries(${PROJECT_NAME}_test PRIVATE
        ${PROJECT_NAME}
        GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(${PROJECT_NAME}_test)
endif()
