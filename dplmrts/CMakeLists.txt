cmake_minimum_required(VERSION 3.6)
project(dplmrts)

find_package(GTest REQUIRED)

add_library(dplmrts
  dplmrts_anytuple.cpp
  dplmrts_invocable.cpp
  dplmrts_invocablearchetype.cpp
)
target_include_directories(dplmrts PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(dplmrts_anytuple.t dplmrts_anytuple.t.cpp)
target_link_libraries(dplmrts_anytuple.t
  dplmrts
  GTest::GTest
)