#!/bin/bash

# Define the checks to perform
#                type     buffer type expected
declare -a CHK01=("i2c" "open-drain")
declare -a CHK02=("i2c" "output-enable")
declare -a CHK03=("i2c" "output-high")
declare -a CHK11=("ps2" "open-drain")
declare -a CHK21=("ksi" "pull-up")

declare -a MCHP_PIN_BUFFER_CHECKS=( \
    # I2C
    "CHK01"\
    # PS2
    "CHK11"\
    # Keyboard matrix
    "CHK21"
)

NPCX_REGEX="[a-z\-]+_npc[a-z0-9]+_card"
MECC_CARD_REGEX="[a-z\-]+_mec+[a-z0-9]+_card"

function perform_buffer_checks() {
    local PLATFORM_NAME=$1
    local BOARD_ID=$2
    local errors=0

    # Other SoC use different property names, support to be added
    if [[ ${PLATFORM_NAME} =~ $MECC_CARD_REGEX ]]; then
        return;
    fi

    if [[ ${PLATFORM_NAME} =~ $NPCX_REGEX ]]; then
        return;
    fi


    echo "Perform device tree checks for current board"
    for i in "${!MCHP_PIN_BUFFER_CHECKS[@]}"; do
        declare -n PIN_BUFFER_CHECK="${MCHP_PIN_BUFFER_CHECKS[i]}"

        local PIN_TYPE=${PIN_BUFFER_CHECK[0]}
        local PROP_NAME=${PIN_BUFFER_CHECK[1]}

        echo "    Checking $PIN_TYPE pins' $PROP_NAME property..."
        local grep_cmd_include="--include=*.dts build"
        local grep_cmd_pin_type_prop_name="grep $PIN_TYPE | grep $PROP_NAME | grep False"
        local grep_cmd="grep -r DT_ ${grep_cmd_include} | ${grep_cmd_pin_type_prop_name}"
        # echo "command to execute: ${grep_cmd}"
        local PINS_CHECKS_RAW=$("grep -r DT_ ${grep_cmd_include} | ${grep_cmd_pin_type_prop_name}")
        if [ " ${PINS_CHECKS_RAW} " != "  " ]; then
            echo "    Not all $PIN_TYPE pins have property $PROP_NAME set to True"
            errors=$((errors + 1))
        fi
    done
    echo "Found ${errors} errors in device tree for EC FW usage"

    if [ $errors -eq 0 ]; then
        echo "All pin buffer settings checks done. No errors!"
    else
        echo "Correct board device tree errors!"
        # TODO: Enforce this check later
        # exit 1
    fi
}
