/* SPDX-License-Identifier: Apache-2.0 */

SECTION_PROLOGUE(_CUSTOM_APP_SECTION_NAME,,ALIGN(32))
{
	KEEP(*(".ecfw_info.*"));
	KEEP(*(".softstrap.*"));
} GROUP_LINK_IN(ROMABLE_REGION)
