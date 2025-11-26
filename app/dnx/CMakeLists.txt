# SPDX-License-Identifier: Apache-2.0

target_sources_ifdef(CONFIG_DNX_SUPPORT app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/dnx.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/dnx.h
    )

target_sources_ifdef(CONFIG_DNX_EC_ASSISTED_TRIGGER app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/dnx_ec_assisted_trigger.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/dnx_ec_assisted_trigger.h
    )
