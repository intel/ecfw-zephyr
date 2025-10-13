#!/bin/bash

function generate_ecfw_binary()
{
	# Location of SPI layout configuration
	NPCX_SPI_CFG=$1
	PROJECT_BINARY=$2
	SPI_IMAGE_BIN=$3

	# Validate parameters
	if [ -f "$NPCX_SPI_CFG" ]; then
		echo "SPI layout configuration: $NPCX_SPI_CFG"
	else
		echo "$NPCX_SPI_CFG does not exist"
		return -1
	fi

	if [ -f "$NPCX_SPI_CFG" ]; then
		echo "Zephyr input binary:  $PROJECT_BINARY"
	else
		echo "$PROJECT_BINARY does not exist"
		return -1
	fi
	echo "SPI output binary:  $SPI_IMAGE_BIN"

	# Extract parameters from SPI layout configuration file
	local ec_region_start_str=$(sed -n -e 's/ECOffsetBytes = //p' $NPCX_SPI_CFG || -1)
	local ec_region_size_str=$(sed -n -e 's/ECRegionSizeKB = //p' $NPCX_SPI_CFG || -1)
	local ec_size_str=$(sed -n -e 's/ECFWSizeKB = //p' $NPCX_SPI_CFG || -1)
	local ec_offset0_str=$(sed -n -e 's/ImageLocation0 = //p' $NPCX_SPI_CFG || -1)
	local ec_offset1_str=$(sed -n -e 's/ImageLocation1 = //p' $NPCX_SPI_CFG || -1)

	# Convert values to integers
	local ec_offset0=$((ec_offset0_str))
	local ec_offset1=$((ec_offset1_str))
	local ecfw_image_size_kb=$((ec_size_str))
	local EC_REGION_START=$((ec_region_start_str))
	local EC_REGION_SIZE_KB=$((ec_region_size_str))
	echo "EC REGION START : $ec_region_start_str"
	echo "EC REGION SIZE : $EC_REGION_SIZE_KB KB"
	echo "EC IMAGE SIZE : $ec_size_str KB"
	echo "Offset image 0 : $ec_offset0_str"
	echo "Offset image 1 : $ec_offset1_str"

	local ifwi_offset0=$(expr $ec_offset0 + $EC_REGION_START)
	local ifwi_offset1=$(expr $ec_offset1 + $EC_REGION_START)
	local ifwi_offset1_boundary=$(expr $ifwi_offset1 % 65536)

	if [ $ifwi_offset0 == $EC_REGION_START ] ; then
		echo "Valid offset for primary image $ec_offset0"
	else
		echo "Invalid offset for primary image $ec_offset0 ($ifwi_offset0)"
		return -1;
	fi

	# Validate 2nd offset is 64KB boundary
	if [ $ifwi_offset1_boundary == 0 ] ; then
		echo "Valid offset image $ec_offset1"
	else
		echo "Invalid offset image $ec_offset1 ($ifwi_offset1) - $ifwi_offset1_boundary"
		return -1;
	fi

	# Calculate primary partition size
	local EC_REGION_SIZE=$(expr $EC_REGION_SIZE_KB \* 1024)
	local primary_partition=$(expr $ec_offset1 - $ec_offset0)
	local secondary_partition=$(expr $EC_REGION_SIZE - $ec_offset1)
	local primary_partition_kb=$(expr $primary_partition / 1024)
	local secondary_partition_kb=$(expr $secondary_partition / 1024)
	echo "Primary partition : $primary_partition[$primary_partition_kb KB] $ifwi_offset0"
	echo "Redundant partition: $secondary_partition[$secondary_partition_kb KB] $ifwi_offset1"

	# Validate both partitions can allocate the EC FW image size
	if [ $primary_partition -gt $secondary_partition ] ; then
		partition_size=$secondary_partition
	else
		partition_size=$primary_partition
	fi
	local partition_size_kb=$(expr $partition_size / 1024)
	if [ $ecfw_image_size_kb -gt $partition_size_kb ] ; then
		echo "Partition size $partition_size_kb to small for EC image $ecfw_image_size_kb"
		return -1
	fi

	# Truncate to offset 1 in KB
	truncate -s ${primary_partition_kb}K ${PROJECT_BINARY}

	# Copy and duplicate
	cp ${PROJECT_BINARY} ${SPI_IMAGE_BIN}
	cat ${PROJECT_BINARY} >> ${SPI_IMAGE_BIN}

	# Truncate to desired EC region size
	truncate -s ${EC_REGION_SIZE_KB}K ${SPI_IMAGE_BIN}
}

generate_ecfw_binary  $1 $2 $3
