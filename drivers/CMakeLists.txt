target_sources_ifdef(CONFIG_ESPI_PERIPHERAL_8042_KBC app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/ps2kbaux.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/ps2kbaux.h
    )

target_sources_ifdef(CONFIG_KSCAN_EC app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/kbs_matrix.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/kbs_matrix.h
    ${CMAKE_CURRENT_LIST_DIR}/kbs_keymap.h
    )

if (CONFIG_KSCAN_EC)
    if (CONFIG_EC_GTECH_KEYBOARD)
        target_sources(app
        PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/gtech_keymap.c
        )
    endif()
endif()

target_sources_ifdef(CONFIG_POSTCODE_MANAGEMENT app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/max6958.c
    ${CMAKE_CURRENT_LIST_DIR}/port80display.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/max6958.h
    ${CMAKE_CURRENT_LIST_DIR}/port80display.h
    )

target_sources_ifdef(CONFIG_THERMAL_MANAGEMENT app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/thermal_sensor.c
    ${CMAKE_CURRENT_LIST_DIR}/peci_hub.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/thermal_sensor.h
    ${CMAKE_CURRENT_LIST_DIR}/peci_hub.h
    )

target_sources_ifdef(CONFIG_BATTERY_MANAGEMENT app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/battery.c
    ${CMAKE_CURRENT_LIST_DIR}/charger.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/battery.h
    ${CMAKE_CURRENT_LIST_DIR}/charger.h
    )

target_sources_ifdef(CONFIG_IMVP_SUPPORT app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/imvp.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/imvp.h
    )

target_sources(app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/eeprom.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/eeprom.h
    ${CMAKE_CURRENT_LIST_DIR}/gpio_ec.h
    ${CMAKE_CURRENT_LIST_DIR}/fan.h
    ${CMAKE_CURRENT_LIST_DIR}/vci.h
    ${CMAKE_CURRENT_LIST_DIR}/acpi.h
    )

target_sources_ifdef(CONFIG_SOC_FAMILY_MEC app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/gpio_mec15xx.c
    ${CMAKE_CURRENT_LIST_DIR}/fan_mec15xx.c
    ${CMAKE_CURRENT_LIST_DIR}/vci.c
    ${CMAKE_CURRENT_LIST_DIR}/acpi_mec15xx.c
    PUBLIC
    )

target_include_directories(app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    )
