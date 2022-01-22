# Copyright (c) Team CharLS.
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.22...3.22)

project("charls-image-test")

add_subdirectory(charls)

set(CMAKE_CXX_STANDARD 20)

add_executable(charls-image-test "")

target_link_libraries(charls-image-test PRIVATE charls)

target_sources(charls-image-test
  PRIVATE
    "src/main.cpp"
    "src/portable_anymap_file.cpp"
    "src/portable_anymap_file.h"
)

