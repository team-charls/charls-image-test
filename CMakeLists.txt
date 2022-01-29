# Copyright (c) Team CharLS.
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.22...3.22)

project("charls-image-test")

add_subdirectory(charls)

add_executable(charls-image-test "")

if(MSVC)
  # MSVC++ can use C++ modules, but CMake has not yet offical suppor for it.
  # Use C++17 when building with CMake, The MSBuild .vcxproj uses C++20.
  target_compile_features(charls-image-test PRIVATE cxx_std_17)
else()
  target_compile_features(charls-image-test PRIVATE cxx_std_20)
endif()

target_sources(charls-image-test
  PRIVATE
    "src/main.cpp"
    "src/portable_anymap_file.cpp"
    "src/portable_anymap_file.h"
)

target_link_libraries(charls-image-test PRIVATE charls)
