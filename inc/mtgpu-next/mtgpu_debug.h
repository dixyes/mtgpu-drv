/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef _MTGPU_DEBUG_H_
#define _MTGPU_DEBUG_H_

#define MTGPU_DUMPDEBUG_LOG(...)					\
	do								\
	{								\
		if (dump_debug_printf)					\
			dump_debug_printf(dump_debug_file, __VA_ARGS__);\
		else							\
			mtgpu_debug_printf(__VA_ARGS__);			\
	} while (0)

#define DEBUG_WIDTH			(4)

struct _PVRSRV_RGXDEV_INFO_;
struct mtgpu_job_list;

void mtgpu_debug_printf(const char *format, ...);

void mtgpu_debug_dump(struct _PVRSRV_RGXDEV_INFO_ *dev_info,
		      bool verbose,
		      DUMPDEBUG_PRINTF_FUNC *dump_debug_printf,
		      void *dump_debug_file);
void mtgpu_debug_dump_job_list(struct _PVRSRV_RGXDEV_INFO_ *dev_info, struct mtgpu_job_list *job_list,
			       DUMPDEBUG_PRINTF_FUNC *dump_debug_printf, void *dump_debug_file);
int mtgpu_debug_dump_init(void);
void mtgpu_debug_dump_deinit(void);

const char *mtgpu_debug_get_ccb_type_string(int value);
const char *mtgpu_debug_get_node_type_string(int value);
const char *mtgpu_debug_get_cmd_type_string(int cmd);

#endif /* MTGPU_DEBUG_H */
