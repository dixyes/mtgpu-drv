/*************************************************************************/ /*!
@File           km_apphint.h
@Title          Apphint internal header
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Linux kernel AppHint control
@License        Dual MIT/GPLv2

The contents of this file are subject to the MIT license as set out below.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

Alternatively, the contents of this file may be used under the terms of
the GNU General Public License Version 2 ("GPL") in which case the provisions
of GPL are applicable instead of those above.

If you wish to allow use of your version of this file only under the terms of
GPL, and not to allow others to use your version of this file under the terms
of the MIT license, indicate your decision by deleting the provisions above
and replace them with the notice and other provisions required by GPL as set
out in the file called "GPL-COPYING" included in this distribution. If you do
not delete the provisions above, a recipient may use your version of this file
under the terms of either the MIT license or GPL.

This License is also included in this distribution in the file called
"MIT-COPYING".

EXCEPT AS OTHERWISE STATED IN A NEGOTIATED AGREEMENT: (A) THE SOFTWARE IS
PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT; AND (B) IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/ /**************************************************************************/

#ifndef KM_APPHINT_H
#define KM_APPHINT_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "km_apphint_defs.h"
#include "htbuffer_types.h"
#include "device.h"

#define APPHINT_DEVICES_MAX 64

/*
*******************************************************************************
 * AppHint mnemonic data type helper tables
******************************************************************************/
struct apphint_lookup {
	const char *name;
	int value;
};

static const struct apphint_lookup fwt_logtype_tbl[] = {
	{ "trace", 0},
	{ "none", 0}
#if defined(SUPPORT_TBI_INTERFACE)
	, { "tbi", 1}
#endif
};

static const struct apphint_lookup fwt_loggroup_tbl[] = {
	RGXFWIF_LOG_GROUP_NAME_VALUE_MAP
};

static const struct apphint_lookup htb_loggroup_tbl[] = {
#define X(a, b) { #b, HTB_LOG_GROUP_FLAG(a) },
	HTB_LOG_SFGROUPLIST
#undef X
};

static const struct apphint_lookup htb_opmode_tbl[] = {
	{ "droplatest", HTB_OPMODE_DROPLATEST},
	{ "dropoldest", HTB_OPMODE_DROPOLDEST},
	{ "block", HTB_OPMODE_BLOCK}
};

__maybe_unused
static const struct apphint_lookup htb_logmode_tbl[] = {
	{ "all", HTB_LOGMODE_ALLPID},
	{ "restricted", HTB_LOGMODE_RESTRICTEDPID}
};

__maybe_unused
static const struct apphint_lookup timecorr_clk_tbl[] = {
	{ "mono", 0 },
	{ "mono_raw", 1 },
	{ "sched", 2 }
};

/*
*******************************************************************************
 Data types
******************************************************************************/
union apphint_value {
	IMG_UINT64 UINT64;
	IMG_UINT32 UINT32;
	IMG_BOOL BOOL;
	IMG_CHAR *STRING;
};

union apphint_query_action {
	PVRSRV_ERROR (*UINT64)(const PVRSRV_DEVICE_NODE *device,
	                       const void *private_data, IMG_UINT64 *value);
	PVRSRV_ERROR (*UINT32)(const PVRSRV_DEVICE_NODE *device,
	                       const void *private_data, IMG_UINT32 *value);
	PVRSRV_ERROR (*BOOL)(const PVRSRV_DEVICE_NODE *device,
	                     const void *private_data, IMG_BOOL *value);
	PVRSRV_ERROR (*STRING)(const PVRSRV_DEVICE_NODE *device,
	                       const void *private_data, IMG_CHAR **value);
};

union apphint_set_action {
	PVRSRV_ERROR (*UINT64)(const PVRSRV_DEVICE_NODE *device,
	                       const void *private_data, IMG_UINT64 value);
	PVRSRV_ERROR (*UINT32)(const PVRSRV_DEVICE_NODE *device,
	                       const void *private_data, IMG_UINT32 value);
	PVRSRV_ERROR (*BOOL)(const PVRSRV_DEVICE_NODE *device,
	                     const void *private_data, IMG_BOOL value);
	PVRSRV_ERROR (*STRING)(const PVRSRV_DEVICE_NODE *device,
	                       const void *private_data, IMG_CHAR *value);
};

struct apphint_action {
	union apphint_query_action query; /*!< Query callbacks. */
	union apphint_set_action set;     /*!< Set callbacks. */
	const PVRSRV_DEVICE_NODE *device; /*!< Pointer to the device node.*/
	const void *private_data;         /*!< Opaque data passed to `query` and
	                                       `set` callbacks. */
	union apphint_value stored;       /*!< Value of the AppHint. */
	bool free;                        /*!< Flag indicating that memory has been
	                                       allocated for this AppHint and it
	                                       needs to be freed on deinit. */
	bool initialised;                 /*!< Flag indicating if the AppHint has
	                                       been already initialised. */
};

struct apphint_param {
	IMG_UINT32 id;
	APPHINT_DATA_TYPE data_type;
	const void *data_type_helper;
	IMG_UINT32 helper_size;
};

struct apphint_init_data {
	IMG_UINT32 id;			/* index into AppHint Table */
	APPHINT_CLASS class;
	const IMG_CHAR *name;
	union apphint_value default_value;
};

struct apphint_init_data_mapping {
	IMG_UINT32 device_apphint_id;
	IMG_UINT32 modparam_apphint_id;
};

struct apphint_class_state {
	APPHINT_CLASS class;
	IMG_BOOL enabled;
};

struct apphint_work {
	struct work_struct *work;
	union apphint_value new_value;
	struct apphint_action *action;
};

/*
*******************************************************************************
 Initialization / configuration table data
******************************************************************************/
#define UINT32Bitfield UINT32
#define UINT32List UINT32

static const struct apphint_init_data init_data_buildvar[] = {
#define X(a, b, c, d, e) \
	{APPHINT_ID_ ## a, APPHINT_CLASS_ ## c, #a, {.b=d} },
	APPHINT_LIST_BUILDVAR_COMMON
	APPHINT_LIST_BUILDVAR
#undef X
};

static const struct apphint_init_data init_data_modparam[] = {
#define X(a, b, c, d, e) \
	{APPHINT_ID_ ## a, APPHINT_CLASS_ ## c, #a, {.b=d} },
	APPHINT_LIST_MODPARAM_COMMON
	APPHINT_LIST_MODPARAM
#undef X
};

static const struct apphint_init_data init_data_debuginfo[] = {
#define X(a, b, c, d, e) \
	{APPHINT_ID_ ## a, APPHINT_CLASS_ ## c, #a, {.b=d} },
	APPHINT_LIST_DEBUGINFO_COMMON
	APPHINT_LIST_DEBUGINFO
#undef X
};

static const struct apphint_init_data init_data_debuginfo_device[] = {
#define X(a, b, c, d, e) \
	{APPHINT_ID_ ## a, APPHINT_CLASS_ ## c, #a, {.b=d} },
	APPHINT_LIST_DEBUGINFO_DEVICE_COMMON
	APPHINT_LIST_DEBUGINFO_DEVICE
#undef X
};

static const struct apphint_init_data_mapping init_data_debuginfo_device_to_modparams[] = {
#define X(a, b) \
	{APPHINT_ID_ ## a, APPHINT_ID_ ## b},
	APPHINT_LIST_DEBUIGINFO_DEVICE_X_MODPARAM_INIT_COMMON
	APPHINT_LIST_DEBUIGINFO_DEVICE_X_MODPARAM_INIT
#undef X
};

#undef UINT32Bitfield
#undef UINT32List

__maybe_unused static const char NO_PARAM_TABLE[] = {};

static const struct apphint_param param_lookup[] = {
#define X(a, b, c, d, e) \
	{APPHINT_ID_ ## a, APPHINT_DATA_TYPE_ ## b, e, ARRAY_SIZE(e) },
	APPHINT_LIST_ALL
#undef X
};

static const struct apphint_class_state class_state[] = {
#define X(a) {APPHINT_CLASS_ ## a, APPHINT_ENABLED_CLASS_ ## a},
	APPHINT_CLASS_LIST
#undef X
};

/*
*******************************************************************************
 Global state
******************************************************************************/
/* If the union apphint_value becomes such that it is not possible to read
 * and write atomically, a mutex may be desirable to prevent a read returning
 * a partially written state.
 * This would require a statically initialized mutex outside of the
 * struct apphint_state to prevent use of an uninitialized mutex when
 * module_params are provided on the command line.
 *     static DEFINE_MUTEX(apphint_mutex);
 */
struct apphint_state
{
	struct workqueue_struct *workqueue;
	DI_GROUP *debuginfo_device_rootdir[APPHINT_DEVICES_MAX];
	DI_ENTRY *debuginfo_device_entry[APPHINT_DEVICES_MAX][APPHINT_DEBUGINFO_DEVICE_ID_MAX];
	DI_GROUP *debuginfo_rootdir;
	DI_ENTRY *debuginfo_entry[APPHINT_DEBUGINFO_ID_MAX];
	DI_GROUP *buildvar_rootdir;
	DI_ENTRY *buildvar_entry[APPHINT_BUILDVAR_ID_MAX];

	unsigned int num_devices;
	PVRSRV_DEVICE_NODE *devices[APPHINT_DEVICES_MAX];
	bool devices_used[APPHINT_DEVICES_MAX];
	unsigned int initialized;

	/* Array contains value space for 1 copy of all apphint values defined
	 * (for device 1) and N copies of device specific apphint values for
	 * multi-device platforms.
	 */
	struct apphint_action val[APPHINT_ID_MAX + ((APPHINT_DEVICES_MAX-1)*APPHINT_DEBUGINFO_DEVICE_ID_MAX)];
};

int pvr_apphint_init(void);
void pvr_apphint_deinit(void);
int apphint_kparam_set(const char *val, const struct kernel_param *kp);
int apphint_kparam_get(char *buffer, const struct kernel_param *kp);
int pvr_apphint_device_register(PVRSRV_DEVICE_NODE *device);
void pvr_apphint_device_unregister(PVRSRV_DEVICE_NODE *device);
void pvr_apphint_dump_state(PVRSRV_DEVICE_NODE *device);

void pvr_apphint_get_bool_from_fwif_config(PVRSRV_DEVICE_NODE *device, IMG_UINT32 ui32Flag, IMG_BOOL *pVal);
int pvr_apphint_get_uint64(PVRSRV_DEVICE_NODE *device, APPHINT_ID ue, IMG_UINT64 *pVal);
int pvr_apphint_get_uint32(PVRSRV_DEVICE_NODE *device, APPHINT_ID ue, IMG_UINT32 *pVal);
int pvr_apphint_get_bool(PVRSRV_DEVICE_NODE *device, APPHINT_ID ue, IMG_BOOL *pVal);
int pvr_apphint_get_string(PVRSRV_DEVICE_NODE *device, APPHINT_ID ue, IMG_CHAR *pBuffer, size_t size);

int pvr_apphint_set_uint64(PVRSRV_DEVICE_NODE *device, APPHINT_ID ue, IMG_UINT64 Val);
int pvr_apphint_set_uint32(PVRSRV_DEVICE_NODE *device, APPHINT_ID ue, IMG_UINT32 Val);
int pvr_apphint_set_bool(PVRSRV_DEVICE_NODE *device, APPHINT_ID ue, IMG_BOOL Val);
int pvr_apphint_set_string(PVRSRV_DEVICE_NODE *device, APPHINT_ID ue, IMG_CHAR *pBuffer, size_t size);

void pvr_apphint_register_handlers_uint64(APPHINT_ID id,
	PVRSRV_ERROR (*query)(const PVRSRV_DEVICE_NODE *device, const void *private_data, IMG_UINT64 *value),
	PVRSRV_ERROR (*set)(const PVRSRV_DEVICE_NODE *device, const void *private_data, IMG_UINT64 value),
	const PVRSRV_DEVICE_NODE *device,
	const void * private_data);
void pvr_apphint_register_handlers_uint32(APPHINT_ID id,
	PVRSRV_ERROR (*query)(const PVRSRV_DEVICE_NODE *device, const void *private_data, IMG_UINT32 *value),
	PVRSRV_ERROR (*set)(const PVRSRV_DEVICE_NODE *device, const void *private_data, IMG_UINT32 value),
	const PVRSRV_DEVICE_NODE *device,
	const void *private_data);
void pvr_apphint_register_handlers_bool(APPHINT_ID id,
	PVRSRV_ERROR (*query)(const PVRSRV_DEVICE_NODE *device, const void *private_data, IMG_BOOL *value),
	PVRSRV_ERROR (*set)(const PVRSRV_DEVICE_NODE *device, const void *private_data, IMG_BOOL value),
	const PVRSRV_DEVICE_NODE *device,
	const void *private_data);
void pvr_apphint_register_handlers_string(APPHINT_ID id,
	PVRSRV_ERROR (*query)(const PVRSRV_DEVICE_NODE *device, const void *private_data, IMG_CHAR **value),
	PVRSRV_ERROR (*set)(const PVRSRV_DEVICE_NODE *device, const void *private_data, IMG_CHAR *value),
	const PVRSRV_DEVICE_NODE *device,
	const void *private_data);

#if defined(__cplusplus)
}
#endif
#endif /* KM_APPHINT_H */

/******************************************************************************
 End of file (km_apphint.h)
******************************************************************************/
