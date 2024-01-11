# SPDX-License-Identifier: Apache-2.0

# To build all supported boards
# ./build_all_boards.sh

export CI_PROJECT_DIR=${PWD}
export CI_EC_SCRIPTS=.ci-functions
export ARTIFACTS_FOLDER=binaries

# Build binaries
rm -rf ${ARTIFACTS_FOLDER}
export ARTIFACTS="TRUE"
source ${CI_EC_SCRIPTS}/build.sh
set_artifacts_folder
build_boards ${ARTIFACTS}

unset CI_PROJECT_DIR
unset CI_EC_SCRIPTS
unset ARTIFACTS_FOLDER
