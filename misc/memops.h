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
void __write_size_error(void)
	__attribute__((__error__("write beyond size of object")));
void __read_size_error(void)
	__attribute__((__error__("read beyond size of object")));

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
		__write_size_error();
		return -EINVAL;
	}

	if (__builtin_constant_p(len) && src_sz < len) {
		__read_size_error();
		return -EINVAL;
	}

#if defined(CONFIG_DEBUG_OPTIMIZATIONS)
	memcpy(dst, src, len);
#else
	__builtin___memcpy_chk(dst, src, len, dst_sz);
#endif

	return 0;
}

/**
 * @brief Secure memory set implementation.
 *
 * @retval -EFAULT  source/destination are NULL.
 * @retval -EINVAL if Buffer overflow.
 * @retval 0 if success.
 */
static inline int memsets(void *dst, int c, int len)
{
	if (dst == NULL) {
		return -EFAULT;
	}

	size_t dst_sz = __builtin_object_size(dst, 0);

	if (__builtin_constant_p(len) && dst_sz < len) {
		__write_size_error();
		return -EINVAL;
	}

#if defined(CONFIG_DEBUG_OPTIMIZATIONS)
	memset(dst, c, len);
#else
	__builtin___memset_chk(dst, c, len, dst_sz);
#endif

	return 0;
}

#else
static inline int memcpys(void *dst, const void *src, int len)
{
	memcpy(dst, src, len);
	return 0;
}

static inline int memsets(void *dst, int c, int len)
{
	memset(dst, c, len);
	return 0;
}
#endif
#endif /* __MEMOPS__ */


