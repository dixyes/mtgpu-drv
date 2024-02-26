/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __MTJPU_BUF_H__
#define __MTJPU_BUF_H__

struct jpu_buffer {
	unsigned int gem_handle;
	unsigned int mem_group_id;
	unsigned int mem_types;
	unsigned long long dev_addr;
	unsigned long long cpu_addr;
	unsigned long long kernel_addr;
	unsigned long long user_addr;
	unsigned int size;
};

#endif /* __MTJPU_BUF_H__ */
