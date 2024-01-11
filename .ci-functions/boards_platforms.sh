# SPDX-License-Identifier: Apache-2.0
#!/bin/bash

#               Platform (folder name)  Board config               Additional cfg
declare -a R01=("adl_p"			"mec1501_adl_p"			"")
declare -a R04=("mtl_p"			"mec1501_mtl_p"			"")
declare -a R05=("mtl_s"			"mec172x_mtl_s"			"")

#               Platform (folder name)  Board config               Additional cfg
declare -a S03=("mtlp_mec172x_card"	"mec172xmodular_assy6930"	"")

declare -a SUPPORTED_PLATFORMS=( \
	# Regular RVP
	"R01" "R04" "R05" \
	# MECC enablement
	"S03"
	# Special binaries
	)
