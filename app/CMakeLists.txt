target_sources(app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/app.c
    )

include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app/peripheral_management)
include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app/power_sequencing)
include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app/power_management)
include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app/smchost)
include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app/thermal_management)
include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app/battery_management)
include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app/debug)
include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app/kbchost)
include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app/usbc_management)

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
    ${CMAKE_CURRENT_LIST_DIR}/power_sequencing/espi_hub.c
    ${CMAKE_CURRENT_LIST_DIR}/power_sequencing/dswmode.c
    ${CMAKE_CURRENT_LIST_DIR}/power_sequencing/deepsx.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/power_sequencing/pwrplane.h
    ${CMAKE_CURRENT_LIST_DIR}/power_sequencing/espi_hub.h
    ${CMAKE_CURRENT_LIST_DIR}/power_sequencing/dswmode.h
    ${CMAKE_CURRENT_LIST_DIR}/power_sequencing/deepsx.h
    )

target_sources_ifdef(CONFIG_PWRMGMT_DEEP_IDLE app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/power_management/pwrmgmt.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/power_management/pwrmgmt.h
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

target_sources_ifdef(CONFIG_PFAT_SUPPORT app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/smchost/smchost_pfat.c
    )

target_sources_ifdef(CONFIG_THERMAL_MANAGEMENT app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/thermal_management/thermalmgmt.c
    ${CMAKE_CURRENT_LIST_DIR}/smchost/smchost_thermal.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/thermal_management/thermalmgmt.h
    )

target_sources_ifdef(CONFIG_BATTERY_MANAGEMENT app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/battery_management/bmc.c
    ${CMAKE_CURRENT_LIST_DIR}/smchost/smchost_bmc.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/battery_management/bmc.h
    )

target_sources_ifdef(CONFIG_POSTCODE_MANAGEMENT app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/debug/postcodemgmt.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/debug/postcodemgmt.h
    )


target_sources_ifdef(CONFIG_USBC_POWER_DELIVERY app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/usbc_management/usbc.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/usbc_management/usbc.h
    )

target_sources_ifdef(CONFIG_ESPI_PERIPHERAL_8042_KBC app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/kbchost/kbchost.c
    ${CMAKE_CURRENT_LIST_DIR}/kbchost/keyboard_utility.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/kbchost/kbchost.h
    ${CMAKE_CURRENT_LIST_DIR}/kbchost/keyboard_utility.h
    )

# Modules not open sourced
target_sources_ifdef(CONFIG_ESPI_PERIPHERAL_HOST_IO_PVT app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/smchost_pvt.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/smchost_pvt.h
    )

target_include_directories(app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    )
