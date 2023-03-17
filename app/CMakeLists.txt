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

#

target_sources_ifdef(CONFIG_PERIPHERAL_MANAGEMENT app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/peripheral_management/periphmgmt.c
    ${CMAKE_CURRENT_LIST_DIR}/peripheral_management/pwrbtnmgmt.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/peripheral_management/periphmgmt.h
    ${CMAKE_CURRENT_LIST_DIR}/peripheral_management/pwrbtnmgmt.h
    )

target_sources_ifdef(CONFIG_PWRMGMT app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/power_sequencing/pwrplane.c
    ${CMAKE_CURRENT_LIST_DIR}/power_sequencing/dswmode.c
    ${CMAKE_CURRENT_LIST_DIR}/power_sequencing/deepsx.c
    ${CMAKE_CURRENT_LIST_DIR}/power_sequencing/pseudog3.c
    ${CMAKE_CURRENT_LIST_DIR}/power_sequencing/pmc.c
    ${CMAKE_CURRENT_LIST_DIR}/power_sequencing/pwrseq_utils.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/power_sequencing/pwrplane.h
    ${CMAKE_CURRENT_LIST_DIR}/power_sequencing/dswmode.h
    ${CMAKE_CURRENT_LIST_DIR}/power_sequencing/deepsx.h
    ${CMAKE_CURRENT_LIST_DIR}/power_sequencing/pseudog3.h
    ${CMAKE_CURRENT_LIST_DIR}/power_sequencing/pmc.h
    ${CMAKE_CURRENT_LIST_DIR}/power_sequencing/pwrseq_utils.h
    )

target_sources_ifdef(CONFIG_DNX_SUPPORT app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/dnx/dnx.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/dnx/dnx.h
    )

target_sources_ifdef(CONFIG_DNX_EC_ASSISTED_TRIGGER app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/dnx/dnx_ec_assisted_trigger.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/dnx/dnx_ec_assisted_trigger.h
    )

target_sources_ifdef(CONFIG_SOC_DEBUG_AWARENESS app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/soc_debug_awareness/soc_debug.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/soc_debug_awareness/soc_debug.h
    )

target_sources_ifdef(CONFIG_SMCHOST app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/smchost/smchost.c
    ${CMAKE_CURRENT_LIST_DIR}/smchost/smchost_info.c
    ${CMAKE_CURRENT_LIST_DIR}/smchost/smchost_pm.c
    ${CMAKE_CURRENT_LIST_DIR}/smchost/smc.c
    ${CMAKE_CURRENT_LIST_DIR}/smchost/sci.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/smchost/smchost.h
    ${CMAKE_CURRENT_LIST_DIR}/smchost/smc.h
    ${CMAKE_CURRENT_LIST_DIR}/smchost/sci.h
    ${CMAKE_CURRENT_LIST_DIR}/smchost/scicodes.h
    )

target_sources_ifdef(CONFIG_THERMAL_MANAGEMENT app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/thermal_management/thermalmgmt.c
    ${CMAKE_CURRENT_LIST_DIR}/smchost/smchost_thermal.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/thermal_management/thermalmgmt.h
    )

target_sources_ifdef(CONFIG_POSTCODE_MANAGEMENT app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/debug/postcodemgmt.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/debug/postcodemgmt.h
    )

target_sources_ifdef(CONFIG_ESPI_PERIPHERAL_8042_KBC app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/kbchost/kbchost.c
    ${CMAKE_CURRENT_LIST_DIR}/kbchost/keyboard_utility.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/kbchost/kbchost.h
    ${CMAKE_CURRENT_LIST_DIR}/kbchost/keyboard_utility.h
    )

target_sources_ifdef(CONFIG_ESPI_SAF app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/saf/saf_config.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/saf/saf_config.h
    )

target_sources_ifdef(CONFIG_SAF_SPI_WINBOND app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/saf/saf_spi_winbond.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/saf/saf_spi_winbond.h
    )

target_include_directories(app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    )
