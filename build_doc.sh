# SPDX-License-Identifier: Apache-2.0

# To build documentation
# ./build_doc.sh

export CI_PROJECT_DIR=${PWD}
export CI_EC_SCRIPTS=.ci-functions
export DOCUMENTATION_FOLDER=docs

# Build documentation
rm -rf ${CI_PROJECT_DIR}/html
sphinx-build -b html doc html
rm -rf ${CI_PROJECT_DIR}/html/_sources/
rm -rf ${CI_PROJECT_DIR}/html/.doctrees/
cp -r ${CI_PROJECT_DIR}/html/* ${CI_PROJECT_DIR}/docs/
cp ${CI_PROJECT_DIR}/doc/.nojekyll ${CI_PROJECT_DIR}/docs
rm -rf ${CI_PROJECT_DIR}/html

unset CI_PROJECT_DIR
unset CI_EC_SCRIPTS
unset DOCUMENTATION_FOLDER
