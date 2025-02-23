/*************************************************************************/ /*!
@File
@Title          Header for Services abstraction layer
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Declaration of an interface layer used to abstract code that
                can be compiled outside of the DDK, potentially in a
                completely different OS.
                All the headers included by this file must also be copied to
                the alternative source tree.
                All the functions declared here must have a DDK implementation
                inside the DDK source tree (e.g. rgxlayer_impl.h/.c) and
                another different implementation in case they are used outside
                of the DDK.
                All of the functions accept as a first parameter a
                "const void *hPrivate" argument. It should be used to pass
                around any implementation specific data required.
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

#if !defined(RGXLAYER_H)
#define RGXLAYER_H

#if defined(__cplusplus)
extern "C" {
#endif


#include "img_defs.h"
#include "img_types.h"
#include "img_elf.h"
#include "pvrsrv_error.h" /* includes pvrsrv_errors.h */
#include "pvrsrv_device.h"
#include "rgx_bvnc_defs_km.h"
#include "rgx_fw_info.h"
#include "rgx_fwif_shared.h" /* includes rgx_common.h and mem_types.h */
#include "rgx_meta.h"
#include "rgx_riscv.h"

#include "rgxdefs_km.h"
/* includes:
 * rgx_cr_defs_km.h,
 * RGX_BVNC_CORE_KM_HEADER (rgxcore_km_B.V.N.C.h),
 * RGX_BNC_CONFIG_KM_HEADER (rgxconfig_km_B.V.N.C.h)
 */


/*!
*******************************************************************************

 @Function       RGXMemCopy

 @Description    MemCopy implementation

 @Input          hPrivate   : Implementation specific data
 @Input          pvDst      : Pointer to the destination
 @Input          pvSrc      : Pointer to the source location
 @Input          uiSize     : The amount of memory to copy in bytes

 @Return         void

******************************************************************************/
void RGXMemCopy(const void *hPrivate,
                void *pvDst,
                void *pvSrc,
                size_t uiSize);

/*!
*******************************************************************************

 @Function       RGXMemSet

 @Description    MemSet implementation

 @Input          hPrivate   : Implementation specific data
 @Input          pvDst      : Pointer to the start of the memory region
 @Input          ui8Value   : The value to be written
 @Input          uiSize     : The number of bytes to be set to ui8Value

 @Return         void

******************************************************************************/
void RGXMemSet(const void *hPrivate,
               void *pvDst,
               IMG_UINT8 ui8Value,
               size_t uiSize);

/*!
*******************************************************************************

 @Function       RGXGetMMUVersion

 @Description    Get mmu version

 @Input          hPrivate   : Implementation specific data

 @Return         IMG_UINT32

******************************************************************************/
IMG_UINT32 RGXGetMMUVersion(const void *hPrivate);

/*!
*******************************************************************************

 @Function       RGXCommentLog

 @Description    Generic log function used for debugging or other purposes

 @Input          hPrivate   : Implementation specific data
 @Input          pszString  : Message to be printed
 @Input          ...        : Variadic arguments

 @Return         void

******************************************************************************/
__printf(2, 3)
void RGXCommentLog(const void *hPrivate,
                   const IMG_CHAR *pszString,
                   ...);

/*!
*******************************************************************************

 @Function       RGXErrorLog

 @Description    Generic error log function used for debugging or other purposes

 @Input          hPrivate   : Implementation specific data
 @Input          pszString  : Message to be printed
 @Input          ...        : Variadic arguments

 @Return         void

******************************************************************************/
__printf(2, 3)
void RGXErrorLog(const void *hPrivate,
                 const IMG_CHAR *pszString,
                 ...);

/* This is used to check if a specific feature is enabled.
 * Should be used instead of calling RGXDeviceHasFeature.  */
#define RGX_DEVICE_HAS_FEATURE(hPrivate, Feature) \
			RGXDeviceHasFeature(hPrivate, RGX_FEATURE_##Feature##_BIT_MASK)

/* This is used to check if a specific feature with value is enabled.
 * Should be used instead of calling RGXDeviceGetFeatureValue.  */
#define RGX_DEVICE_HAS_FEATURE_VALUE(hPrivate, Feature) \
			(RGXDeviceGetFeatureValue(hPrivate, RGX_FEATURE_##Feature##_IDX) >= 0)

/* This is used to get the value of a specific feature from hPrivate.
 * Should be used instead of calling RGXDeviceGetFeatureValue.  */
#define RGX_DEVICE_GET_FEATURE_VALUE(hPrivate, Feature) \
			RGXDeviceGetFeatureValue(hPrivate, RGX_FEATURE_##Feature##_IDX)

/* This is used to get the value of a specific ERN from hPrivate.
 * Should be used instead of calling RGXDeviceHasErnBrn.  */
#define RGX_DEVICE_HAS_ERN(hPrivate, FixNum) \
			RGXDeviceHasErnBrn(hPrivate, HW_##FixNum##_BIT_MASK)

/* This is used to get the value of a specific BRN from hPrivate.
 * Should be used instead of calling RGXDeviceHasErnBrn.  */
#define RGX_DEVICE_HAS_BRN(hPrivate, FixNum) \
			RGXDeviceHasErnBrn(hPrivate, FIX_HW_##FixNum##_BIT_MASK)

#define CLK_CTRL_FORCE_ON(X, Module) \
			X = (((X) & RGX_CR_##Module##_CLRMSK) | RGX_CR_##Module##_ON)
/*!
*******************************************************************************

 @Function       RGXDeviceGetFeatureValue

 @Description    Checks if a device has a particular feature with values

 @Input          hPrivate     : Implementation specific data
 @Input          ui64Feature  : Feature with values to check

 @Return         Value >= 0 if the given feature is available, -1 otherwise

******************************************************************************/
IMG_INT32 RGXDeviceGetFeatureValue(const void *hPrivate, IMG_UINT64 ui64Feature);

/*!
*******************************************************************************

 @Function       RGXDeviceHasFeature

 @Description    Checks if a device has a particular feature

 @Input          hPrivate     : Implementation specific data
 @Input          ui64Feature  : Feature to check

 @Return         IMG_TRUE if the given feature is available, IMG_FALSE otherwise

******************************************************************************/
IMG_BOOL RGXDeviceHasFeature(const void *hPrivate, IMG_UINT64 ui64Feature);

/*!
*******************************************************************************

 @Function       RGXDeviceHasErnBrn

 @Description    Checks if a device has a particular errata

 @Input          hPrivate     : Implementation specific data
 @Input          ui64ErnsBrns : Flags to check

 @Return         IMG_TRUE if the given errata is available, IMG_FALSE otherwise

******************************************************************************/
IMG_BOOL RGXDeviceHasErnBrn(const void *hPrivate, IMG_UINT64 ui64ErnsBrns);

/*!
*******************************************************************************

 @Function       RGXGetFWCorememSize

 @Description    Get the FW coremem size

 @Input          hPrivate   : Implementation specific data

 @Return         FW coremem size

******************************************************************************/
IMG_INTERNAL
IMG_UINT32 RGXGetFWCorememSize(const void *hPrivate);

/*!
*******************************************************************************

 @Function      RGXWriteReg32/64

 @Description   Write a value to a 32/64 bit RGX register

 @Input         hPrivate         : Implementation specific data
 @Input         ui32RegAddr      : Register offset inside the register bank
 @Input         ui32/64RegValue  : New register value

 @Return        void

******************************************************************************/
void RGXWriteReg32(const void *hPrivate,
                   IMG_UINT32 ui32RegAddr,
                   IMG_UINT32 ui32RegValue);

void RGXWriteReg64(const void *hPrivate,
                   IMG_UINT32 ui32RegAddr,
                   IMG_UINT64 ui64RegValue);

/*!
*******************************************************************************

 @Function       RGXReadReg32/64

 @Description    Read a 32/64 bit RGX register

 @Input          hPrivate     : Implementation specific data
 @Input          ui32RegAddr  : Register offset inside the register bank

 @Return         Register value

******************************************************************************/
IMG_UINT32 RGXReadReg32(const void *hPrivate,
                        IMG_UINT32 ui32RegAddr);

IMG_UINT64 RGXReadReg64(const void *hPrivate,
                        IMG_UINT32 ui32RegAddr);

/*!
*******************************************************************************

 @Function       RGXReadModifyWriteReg32

 @Description    Read-modify-write a 32 bit RGX register

 @Input          hPrivate     : Implementation specific data.
 @Input          ui32RegAddr  : Register offset inside the register bank.
 @Input          ui32RegValue : New register value.
 @Input          ui32RegMask  : Keep the bits set in the mask.

 @Return         Always returns PVRSRV_OK

******************************************************************************/
IMG_UINT32 RGXReadModifyWriteReg64(const void *hPrivate,
                                   IMG_UINT32 ui32RegAddr,
                                   IMG_UINT64 ui64RegValue,
                                   IMG_UINT64 ui64RegKeepMask);

/*!
*******************************************************************************

 @Function       RGXPollReg32/64

 @Description    Poll on a 32/64 bit RGX register until some bits are set/unset

 @Input          hPrivate         : Implementation specific data
 @Input          ui32RegAddr      : Register offset inside the register bank
 @Input          ui32/64RegValue  : Value expected from the register
 @Input          ui32/64RegMask   : Only the bits set in this mask will be
                                    checked against uiRegValue

 @Return         PVRSRV_OK if the poll succeeds,
                 PVRSRV_ERROR_TIMEOUT if the poll takes too long

******************************************************************************/
PVRSRV_ERROR RGXPollReg32(const void *hPrivate,
                          IMG_UINT32 ui32RegAddr,
                          IMG_UINT32 ui32RegValue,
                          IMG_UINT32 ui32RegMask);

PVRSRV_ERROR RGXPollReg64(const void *hPrivate,
                          IMG_UINT32 ui32RegAddr,
                          IMG_UINT64 ui64RegValue,
                          IMG_UINT64 ui64RegMask);

/*!
*******************************************************************************

 @Function       RGXWaitCycles

 @Description    Wait for a number of GPU cycles and/or microseconds

 @Input          hPrivate    : Implementation specific data
 @Input          ui32Cycles  : Number of GPU cycles to wait for in pdumps,
                               it can also be used when running driver-live
                               if desired (ignoring the next parameter)
 @Input          ui32WaitUs  : Number of microseconds to wait for when running
                               driver-live

 @Return         void

******************************************************************************/
void RGXWaitCycles(const void *hPrivate,
                   IMG_UINT32 ui32Cycles,
                   IMG_UINT32 ui32WaitUs);

/*!
*******************************************************************************

 @Function        RGXAcquireKernelMMUPC

 @Description     Acquire the Kernel MMU Page Catalogue device physical address

 @Input           hPrivate  : Implementation specific data
 @Input           psPCAddr  : Returned page catalog address

 @Return          void

******************************************************************************/
void RGXAcquireKernelMMUPC(const void *hPrivate, IMG_DEV_PHYADDR *psPCAddr);

/*!
*******************************************************************************

 @Function        RGXWriteKernelMMUPC32/64

 @Description     Write the Kernel MMU Page Catalogue to the 32/64 bit
                  RGX register passed as argument.
                  In a driver-live scenario without PDump these functions
                  are the same as RGXWriteReg32/64 and they don't need
                  to be reimplemented.

 @Input           hPrivate        : Implementation specific data
 @Input           ui32PCReg       : Register offset inside the register bank
 @Input           ui32AlignShift  : PC register alignshift
 @Input           ui32Shift       : PC register shift
 @Input           ui32/64PCVal    : Page catalog value (aligned and shifted)

 @Return          void

******************************************************************************/
#if defined(PDUMP)
void RGXWriteKernelMMUPC32(const void *hPrivate,
                           IMG_UINT32 ui32PCReg,
                           IMG_UINT32 ui32PCRegAlignShift,
                           IMG_UINT32 ui32PCRegShift,
                           IMG_UINT32 ui32PCVal);

#else  /* defined(PDUMP) */
#define RGXWriteKernelMMUPC32(priv, pcreg, alignshift, shift, pcval) \
	RGXWriteReg32(priv, pcreg, pcval)
#endif /* defined(PDUMP) */

/*!
*******************************************************************************

 @Function        RGXWriteKernelMMUPC32/64

 @Description     Write the Kernel MMU Page Catalogue to the 32/64 bit
                  RGX register passed as argument.
                  In a driver-live scenario without PDump these functions
                  are the same as RGXWriteReg32/64 and they don't need
                  to be reimplemented.

 @Input           hPrivate        : Implementation specific data
 @Input           ui32PCReg       : Register offset inside the register bank
 @Input           ui32AlignShift  : PC register alignshift
 @Input           ui32Shift       : PC register shift
 @Input           ui32/64PCVal    : Page catalog value (aligned and shifted)

 @Return          void

******************************************************************************/
#if defined(PDUMP)
void RGXWriteKernelMMUPC64(const void *hPrivate,
                           IMG_UINT32 ui32PCReg,
                           IMG_UINT32 ui32PCRegAlignShift,
                           IMG_UINT32 ui32PCRegShift,
                           IMG_UINT64 ui64PCVal);

#else  /* defined(PDUMP) */
#define RGXWriteKernelMMUPC64(priv, pcreg, alignshift, shift, pcval) \
        RGXWriteReg64(priv, pcreg, pcval)
#endif /* defined(PDUMP) */

/*!
*******************************************************************************

 @Function        RGXDoFWSlaveBoot

 @Description     Returns whether or not a FW Slave Boot is required
                  while powering on

 @Input           hPrivate       : Implementation specific data

 @Return          IMG_BOOL

******************************************************************************/
IMG_BOOL RGXDoFWSlaveBoot(const void *hPrivate);

/*!
*******************************************************************************

 @Function       RGXFabricCoherencyTest

 @Description    Performs fabric coherency test

 @Input          hPrivate         : Implementation specific data

 @Return         PVRSRV_OK if the test succeeds,
                 PVRSRV_ERROR_INIT_FAILURE if the test fails at some point

******************************************************************************/
PVRSRV_ERROR RGXFabricCoherencyTest(const void *hPrivate);

/*!
*******************************************************************************

 @Function       RGXGetDeviceSLCBanks

 @Description    Returns the number of SLC banks used by the device

 @Input          hPrivate    : Implementation specific data

 @Return         Number of SLC banks

******************************************************************************/
IMG_UINT32 RGXGetDeviceSLCBanks(const void *hPrivate);

/*!
*******************************************************************************

 @Function       RGXGetDeviceCacheLineSize

 @Description    Returns the device cache line size

 @Input          hPrivate    : Implementation specific data

 @Return         Cache line size

******************************************************************************/
IMG_UINT32 RGXGetDeviceCacheLineSize(const void *hPrivate);

/*!
*******************************************************************************

 @Function        RGXAcquireBootCodeAddr

 @Description     Acquire the device virtual address of the RISCV boot code

 @Input           hPrivate         : Implementation specific data
 @Output          psBootCodeAddr   : Boot code base address

 @Return          void

******************************************************************************/
void RGXAcquireBootCodeAddr(const void *hPrivate, IMG_DEV_VIRTADDR *psBootCodeAddr);

/*!
*******************************************************************************

 @Function        RGXAcquireBootDataAddr

 @Description     Acquire the device virtual address of the RISCV boot data

 @Input           hPrivate         : Implementation specific data
 @Output          psBootDataAddr   : Boot data base address

 @Return          void

******************************************************************************/
void RGXAcquireBootDataAddr(const void *hPrivate, IMG_DEV_VIRTADDR *psBootDataAddr);

/*!
*******************************************************************************

 @Function      RGXDeviceAckIrq

 @Description   Checks the implementation specific IRQ status register,
                clearing it if necessary and returning the IRQ status.

 @Input          hPrivate    : Implementation specific data

 @Return:       IRQ status

******************************************************************************/
IMG_BOOL RGXDeviceAckIrq(const void *hPrivate);

#if defined(__cplusplus)
}
#endif

#endif /* RGXLAYER_H */
