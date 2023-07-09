/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include "img_types.h"
#include "pvrsrv_error.h"

typedef struct _MEMORY_POOL_ MEMORY_POOL;

/**
 * PFN_IMPORT_ALLOC - Callback function when memory pool alloc failed
 *
 * @void *:		private handle
 * @IMG_UINT64:		request size
 * @IMG_UINT64 *:	actual size
 * Return reallocated memory address on success, NULL on failure.
 */
typedef void* (*PFN_IMPORT_ALLOC)(void *, IMG_UINT64, IMG_UINT64 *);

/**
 * PFN_IMPORT_FREE - Callback function when address is reallocated
 *
 * @void *:		private handle
 * @IMG_UINT64:		pointer to be freed
 */
typedef void (*PFN_IMPORT_FREE)(void *, void *);

/**
 * MemoryPoolCreate - Create a memory pool
 *
 * @pvBase:		the base address of the buffer
 * @ui64Size:		the buffer size
 * @pfnImportAlloc:	the import alloc function called when alloc failed
 * @pfnImportFree:	the import free function called for address allocated by import alloc
 * @pvPriv:		the private handle for callback function
 *
 * Create a RA handle to manage the buffer. Return a memory pool handle
 * on success, NULL on failure.
 */
MEMORY_POOL *MemoryPoolCreate(void *pvBase,
			      IMG_UINT64 ui64Size,
			      PFN_IMPORT_ALLOC pfnImportAlloc,
			      PFN_IMPORT_FREE pfnImportFree,
			      void *pvPriv);

/**
 * MemoryPoolDelete - Delete a memory pool
 *
 * @psMemPool:		the memory pool handle
 *
 * Delete the RA handle, and free the memory pool struct
 */
void MemoryPoolDelete(MEMORY_POOL *psMemPool);

/**
 * MemoryPoolAlloc - Allocate a memory area from the memory pool
 *
 * @psMemPool:		the memory pool handle
 * @ui32Size:		the alloc size
 *
 * It is recommended that the MemoryPoolAlloc is used when the buffer is needed
 * frequently and will be freed soon. For long-term used allocation please just
 * use system alloc interface directly.
 *
 * Returns the virtual address of the memory on success, NULL if no free memory.
 */
void *MemoryPoolAlloc(MEMORY_POOL *psMemPool, IMG_UINT64 ui64Size);

/**
 * MemoryPoolFree - Free a memory area to the memory pool
 *
 * @psMemPool:		the memory pool handle
 * @pvPtr:		pointer to the memory area
 *
 * Free the memory area if the pointer is allocated by RA or import alloc.
 * Print error message when the pvPtr is invalid.
 */
void MemoryPoolFree(MEMORY_POOL *psMemPool, void *pvPtr);

/* Default import callback function */
void *MemoryPoolImportAllocDefault(void *pvPriv, IMG_UINT64 ui64Size, IMG_UINT64 *puiActualSize);
void MemoryPoolImportFreeDefault(void *pvPriv, void *pvPtr);

/* Alloc/free interface for default global memory pool */
void *MemoryPoolAllocDefault(IMG_UINT64 ui64Size);
void MemoryPoolFreeDefault(void *pvPtr);

/**
 * MemoryPoolInitDefault - Init the default memory pool
 *
 * Returns PVRSRV_OK on success, PVRSRV_ERROR_OUT_OF_MEMORY on failure.
 */
PVRSRV_ERROR MemoryPoolInitDefault(void);

/**
 * MemoryPoolDeInitDefault - Deinit the default memory pool
 */
void MemoryPoolDeInitDefault(void);

#endif /* MEMORY_POOL_H */
