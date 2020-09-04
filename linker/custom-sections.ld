/* SPDX-License-Identifier: Apache-2.0 */

SECTION_PROLOGUE(_CUSTOM_APP_SECTION_NAME,,)
{
	KEEP(*(".ecfw_info.*"));
} GROUP_LINK_IN(ROMABLE_REGION)
