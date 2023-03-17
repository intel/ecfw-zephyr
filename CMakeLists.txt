# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)
set(BOARD_ROOT ${CMAKE_CURRENT_LIST_DIR}/out_of_tree_boards)
include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(ecfw)

include(${CMAKE_CURRENT_LIST_DIR}/app/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/boards/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/drivers/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/misc/CMakeLists.txt)

#Note : The target named "app" is the target name expected by Zephyr RTOS
include_directories(ecfw_app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

zephyr_compile_options(-Werror -Wno-address-of-packed-member)
zephyr_linker_sources(SECTIONS linker/custom-sections.ld)
