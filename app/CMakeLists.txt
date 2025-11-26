# SPDX-License-Identifier: Apache-2.0

target_sources(app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/app.c
    )

include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app/peripheral_management)
include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app/power_sequencing)
include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app/dnx)
include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app/power_management)
include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app/smchost)
include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app/thermal_management)
include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app/debug)
include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app/kbchost)
include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app/saf)
include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app/soc_debug_awareness)

include(${CMAKE_CURRENT_LIST_DIR}/peripheral_management/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/power_sequencing/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/dnx/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/soc_debug_awareness/CMakeLists.txt)

include(${CMAKE_CURRENT_LIST_DIR}/smchost/CMakeLists.txt)

include(${CMAKE_CURRENT_LIST_DIR}/thermal_management/CMakeLists.txt)

include(${CMAKE_CURRENT_LIST_DIR}/debug/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/kbchost/CMakeLists.txt)


include(${CMAKE_CURRENT_LIST_DIR}/saf/CMakeLists.txt)

target_include_directories(app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    )
