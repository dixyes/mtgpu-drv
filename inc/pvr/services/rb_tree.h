/*
 * @Copyright Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License Dual MIT/GPLv2
 */

#ifndef RB_TREE_H
#define RB_TREE_H

#include "pvrsrv_error.h"
#include "img_types.h"

typedef struct __RB_NODE__ RB_NODE;
typedef struct __RB_TREE__ RB_TREE;

typedef PVRSRV_ERROR (*RB_TREE_PFN_CALLBACK) (RB_NODE *, void *);

IMG_UINT64 RB_NODE_Key_Start(RB_NODE *psNode);
IMG_UINT64 RB_NODE_Key_End(RB_NODE *psNode);
void *RB_NODE_Value(RB_NODE *psNode);

/*************************************************************************/ /*!
@Function       RB_TREE_Search
@Description    Search a node in rb tree which contains key between
                ui64Start and ui64End.
@Input          psTree        The rb tree.
@Input          ui64Key       The key to be searched.
@Return         NULL if the key is missing, or the node contains the key.
*/ /**************************************************************************/
RB_NODE * RB_TREE_Search(RB_TREE *psTree, IMG_UINT64 ui64Key);

/*************************************************************************/ /*!
@Function       RB_TREE_Insert
@Description    Insert a new node into rb tree with key between ui64Start and
                ui64End.
@Input          psTree        The rb tree.
@Input          ui64Start     The key start.
@Input          ui64End       The key end.
@Input          pvValue       The value.
@Return         NULL if the key is invalid, or the new node.
*/ /**************************************************************************/
RB_NODE * RB_TREE_Insert(RB_TREE *psTree, IMG_UINT64 ui64Start, IMG_UINT64 ui64End, void *pvValue);

/*************************************************************************/ /*!
@Function       RB_TREE_Remove
@Description    Remove a node contains key.
@Input          psTree        The rb tree.
@Input          ui64Key       The key.
@Return         PVRSRV_OK or error code.
*/ /**************************************************************************/
PVRSRV_ERROR RB_TREE_Remove(RB_TREE *psTree, IMG_UINT64 ui64Key);

/*************************************************************************/ /*!
@Function       RB_TREE_Create
@Description    Create an empty rb red black tree with
                [uint64, uint64] as key, void * as pvValue.
@Return         NULL or rb tree handle.
*/ /**************************************************************************/
RB_TREE * RB_TREE_Create(void);

/*************************************************************************/ /*!
@Function       RB_TREE_Delete
@Description    Delete a rb tree created by RB_TREE_Create.
                All nodes in the tree will be freed.
@Input          psTree        rb tree
*/ /**************************************************************************/
void RB_TREE_Delete(RB_TREE *psTree);

/*************************************************************************/ /*!
@Function       RB_TREE_Iterate
@Description    Iterate over every node in the rb tree.
@Input          psTree        rb tree to iterate.
@Input          pfnCallback   Callback to call with the node and pvPriv for each
                              node in the rb tree
@Input          pvPriv        private data.
@Return         Callback error if any, otherwise PVRSRV_OK
*/ /**************************************************************************/
PVRSRV_ERROR RB_TREE_Iterate(RB_TREE *psTree, RB_TREE_PFN_CALLBACK pfnCallback, void *pvPriv);

#endif
