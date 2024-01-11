# SPDX-License-Identifier: Apache-2.0
#!/bin/bash

source ${CI_EC_SCRIPTS}/boards_platforms.sh
source ${CI_EC_SCRIPTS}/misc.sh

function config_build_env {
    cd ${CI_PROJECT_DIR}
    west -V
    west init -l
    west -v update
    # Check if ZEPHYR is already set
    if [ -z ${ZEPHYR_BASE} ]; then
        cd ../ecfwwork/zephyr_fork && ZEPHYR_PATH=${PWD}
    fi
    cd ${ZEPHYR_PATH} && source zephyr-env.sh
    west config --global zephyr.base-prefer env
    # We go back to the app  directory for the next steps
    cd ${CI_PROJECT_DIR}
}

function set_artifacts_folder {
    # Create artifacts folder for each supported board
    mkdir ${CI_PROJECT_DIR}/${ARTIFACTS_FOLDER}
    cd ${CI_PROJECT_DIR}/${ARTIFACTS_FOLDER}/

    for board in ${SUPPORTED_PLATFORMS[@]}; do
        declare -n PLATFORM_INFO="$board"
        local platform_name=${PLATFORM_INFO[0]}
        mkdir ${platform_name}
        mkdir ${platform_name}/release
        mkdir ${platform_name}/debug
    done
}

function save_binaries() {
    local PLATFORM_NAME=$1
    local BOARD_ID=$2
    local FOLDER_NAME=$3
    local DEST_PATH=${ARTIFACTS_PATH}/${PLATFORM_NAME}/${FOLDER_NAME}/

    # Generate binary version suffix from source code
    local MAJOR_VER=$(sed -n -e 's/#define.*MAJOR_VER //p' \
        ${CI_PROJECT_DIR}/misc/flashhdr.c)
    local MINOR_VER=$(sed -n -e 's/#define.*MINOR_VER //p' \
        ${CI_PROJECT_DIR}/misc/flashhdr.c)
    local PATCH_ID=$(sed -n -e 's/#define.*PATCH_ID //p' \
        ${CI_PROJECT_DIR}/misc/flashhdr.c)
    local QS_BUILD_VER=$(sed -n -e 's/#define.*QS_BUILD_VER //p' \
        ${CI_PROJECT_DIR}/misc/flashhdr.c)
    local BIN_SUFFIX=$(printf "%s_v%d.%02d.%02d.%02d" \
        ${PLATFORM_NAME} ${MAJOR_VER} ${MINOR_VER} ${PATCH_ID} ${QS_BUILD_VER})

    if [ "debug" = "${FOLDER_NAME}" ]; then
        # Save debug binaries
        cp -v build/zephyr/zephyr.* ${DEST_PATH}
    fi

    # Board definition is from public tree where ksc is not used
    MECC_CARD_REGEX="[a-z\-]+_mec+[a-z0-9]+_card"
    if [[ ${PLATFORM_NAME} =~ $MECC_CARD_REGEX ]]; then
        cp -v build/zephyr/spi_image.bin ${DEST_PATH}/ECFW_${BIN_SUFFIX}.bin
    else
        cp -v build/zephyr/ksc.bin ${DEST_PATH}/ECFW_${BIN_SUFFIX}.bin
    fi
}

function build_boards() {
    local ARTIFACTS="$1"
    local ARTIFACTS_PATH="${CI_PROJECT_DIR}/${ARTIFACTS_FOLDER}"
    local REPORT_ERROR=0

    if [ "${ARTIFACTS}" = "TRUE" ] && \
        [ ! -d ${CI_PROJECT_DIR}/${ARTIFACTS_FOLDER} ]; then
        error "ERROR: You must create the artifacts folder first"
        exit 1
    fi

    cd ${CI_PROJECT_DIR}

    mkdir build

    for i in "${!SUPPORTED_PLATFORMS[@]}"; do
        start_build=$(date +%s)
        printf '%s\t%s\n' "${SUPPORTED_PLATFORMS[i]}"
        declare -n PLATFORM_INFO="${SUPPORTED_PLATFORMS[i]}"

        # Need to clean directory before switching boards
        rm -rf build/*

        # Build using west
        local PLATFORM_NAME=${PLATFORM_INFO[0]}
        local BOARD_ID=${PLATFORM_INFO[1]}
        local EXTRA_CONFIG=${PLATFORM_INFO[2]}

        # No logs in release binaries, no JTAG support
        local RELEASE_CONFIG="-DOVERLAY_CONFIG=release.conf ${EXTRA_CONFIG}"
        local DEBUG_CONFIG="-DOVERLAY_CONFIG=debug.conf ${EXTRA_CONFIG}"

        prepare_spi_cfg $PLATFORM_NAME $BOARD_ID

        info "Building release binary for board: ${PLATFORM_NAME}\n"
        debug "west build -c -p always -b ${BOARD_ID} -- -G'Unix Makefiles' ${RELEASE_CONFIG}"
        west build -o=-j4 -c -p always -b ${BOARD_ID} -- -G'Unix Makefiles' \
            "${RELEASE_CONFIG}"

        if [ "${ARTIFACTS}" = "TRUE" ]; then
            save_binaries ${PLATFORM_NAME} ${BOARD_ID} "release"
        fi

        # Generate debug binaries only in stages where artifacts are saved
        # Custom logs on debug binaries, JTAG support
        if [ "${ARTIFACTS}" = "TRUE" ]; then
            rm -rf build/*

            printf "Debug configuration: ${DEBUG_CONFIG}\n"
            info "Building debug binary for board: ${PLATFORM_NAME}\n"
            debug "west build -c -p always -b ${BOARD_ID} -- -- -G'Unix Makefiles' ${DEBUG_CONFIG}"
            west build -o=-j4 -c -p always -b ${BOARD_ID} -- -G'Unix Makefiles' \
                "${DEBUG_CONFIG}"

            debug "Save binaries for ${PLATFORM_NAME}\n"
            save_binaries ${PLATFORM_NAME} ${BOARD_ID} "debug"
        fi
        end_build=$(date +%s)
        echo "Board build time: $(($end_build-$start_build)) seconds"
    done
}
