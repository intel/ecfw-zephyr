/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __MEMOPS__
#define __MEMOPS__

#include <string.h>

#if !defined(CONFIG_NO_OPTIMIZATIONS) && defined(_FORTIFY_SOURCE) \
	&& _FORTIFY_SOURCE > 1 || defined(CONFIG_SPEED_OPTIMIZATIONS)
/**
 * @brief Secure memory copy implementation.
 *
 * @retval -EFAULT  source/destination are NULL.
 * @retval -EINVAL if Buffer overflow.
 * @retval 0 if success.
 */
static inline int memcpys(void *dst, const void *src, int len)
{
	if (dst == NULL || src == NULL) {
		return -EFAULT;
	}

	size_t dst_sz = __builtin_object_size(dst, 0);
	size_t src_sz = __builtin_object_size(src, 0);

	if (__builtin_constant_p(len) && dst_sz < len) {
		return -EINVAL;
	}

	if (__builtin_constant_p(len) && src_sz < len) {
		return -EINVAL;
	}

	__builtin___memcpy_chk(dst, src, len, dst_sz);

	return 0;
}

#endif
#endif /* __MEMOPS__ */


