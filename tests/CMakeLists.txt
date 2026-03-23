cmake_minimum_required(VERSION 3.8)

project(lullaby_tests CXX)

option(BUILD_GTEST "Download and build googletest" ON)

if(BUILD_GTEST)
  include(FetchContent)
  # When building googletest on MSVC ensure it uses the shared CRT to match
  # the default Visual Studio configuration for this project. This prevents
  # linker errors caused by mixing static and dynamic CRTs.
  if(MSVC)
    set(gtest_force_shared_crt ON CACHE BOOL "Use shared CRT for googletest" FORCE)
  endif()
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/heads/main.zip
  )
  FetchContent_MakeAvailable(googletest)
endif()

find_package(Threads REQUIRED)

enable_testing()

add_executable(lullaby_tests test_integration_musicservice.cpp)
target_include_directories(lullaby_tests PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(lullaby_tests PRIVATE gtest_main lullaby_core Threads::Threads)

include(GoogleTest)
gtest_discover_tests(lullaby_tests)
