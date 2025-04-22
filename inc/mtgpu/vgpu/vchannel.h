/*
 * @Copyright   Copyright (c) Moorethreads Technologies Ltd. All Rights Reserved
 * @License     Dual MIT/GPLv2
 */

#ifndef __VCHANNEL_H__
#define __VCHANNEL_H__

#ifdef __linux__
#include "mtgpu_drv.h"
#else
#include "basic_defs.h"
#endif

#include "vgpu_shared_ipc.h"

#define VQUEUE_MAX_QUEUE_SIZE (15)

/** 
 * VQUEUE_MAX_COUNT is defined as one more than VQUEUE_MAX_QUEUE_SIZE to 
 * reserve extra space to help determine the situation when the circular 
 * queue is full. 
 */
#define VQUEUE_MAX_COUNT      (VQUEUE_MAX_QUEUE_SIZE + 1)

/* vchannel errno definition */
#define VCHANNEL_SUCCESS      (0)
#define VCHANNEL_EINVAL       (-1)
#define VCHANNEL_ENOMEM       (-2)
#define VCHANNEL_EBUSY        (-3)
#define VCHANNEL_ENOTSUPP     (-4)
#define VCHANNEL_EGUESTTMOUT  (-5)
#define VCHANNEL_NOT_READY    (-6)
#define VCHANNEL_EOF          (-7)
#define VCHANNEL_ENOENT       (-8)
#define VCHANNEL_EIO          (-9)
#define VCHANNEL_BS_NOT_READY (-10)

typedef enum vqueue_direction {
	/* vqueues designed for sending write and read request. */
	VQUEUE_DIRECTION_GUEST_TO_HOST_RW,
	VQUEUE_DIRECTION_HOST_TO_GUEST_RW,

	/* vqueues specifically designed for sending read ack. */
	VQUEUE_DIRECTION_GUEST_TO_HOST_ACK,
	VQUEUE_DIRECTION_HOST_TO_GUEST_ACK,

	VQUEUE_DIRECTION_MAX_COUNT,
} vqueue_direction_t;

typedef enum vchannel_bs_direction {
	VCHANNEL_BS_DIRECTION_GUEST_TO_HOST_RD,
	VCHANNEL_BS_DIRECTION_HOST_TO_GUEST_RD,

	VCHANNEL_BS_DIRECTION_MAX_COUNT,
} vchannel_bs_direction_t;

struct vqueue_msg {
	uint64_t data;
	/**
	 * Message major type grouping, used to classify
	 * a set of related messages with strong associations.
	 */
	uint8_t group;
	/**
	 * Message type, used to differentiate and identify
	 * the specific purpose of a message.
	 */
	uint8_t type;
	/**
	 * This value is specifically used to represent
	 * the status of ack type message. If non-zero,
	 * it indicates that the "data" field of this 
	 * vqueue_msg is invalid.
	 *
	 * This value is set to 0 by default. If the receiver
	 * of a read message encounters an error while responding
	 * with an ack, this value should be set to a non-zero
	 * value within the vchannel->ops.read callback function.
	 */
	int8_t ack_status;

	/* Used internally. */
	uint8_t opcode;
	uint32_t __resv1;
	uint64_t __resv2;
	uint64_t __resv3;
};

/* ring buffer between host and guest */
struct vqueue {
	uint8_t wo;       /* write offset */
	uint8_t ro;       /* read offset */
	uint16_t __resv1;
	uint32_t __resv2;
	uint64_t __resv3;
	struct vqueue_msg msgs[VQUEUE_MAX_COUNT];
};

/**
 * struct vchannel_ops - Structure to be registered for both guest and host kernel driver.
 *
 * @setup_vqs:		Called to associate the pointer of the ring buffer
 *			vqueue with the vchannel, facilitating management.
 *			@vqs: ring buffer address of the vchannel (output param)
 *			@priv: kernel driver private data
 * @kick:		Called to notify the message receiver that the message has been sent.
 *			@direction: notify the receiver about which vqueue has been kicked
 *			@priv: kernel driver private data
 * @write:		Handling the write message callback function for the sender.
 *			@msg: message of the write type
 *			@priv: kernel driver private data
 * @read:		Handling the read message callback function for the sender.
 *			@msg: message of the read request type from the sender
 *			@ack_data: reply data for the sender (output param)
 *			@ack_status: reply err status for the sender (output param)
 *			@priv: kernel driver private data
 *
 * @bs_setup:		Called to associate the pointer of the byte stream buffer
 *			with the vchannel, facilitating management.
 *			@buf: byte stream buffer address of the vchannel (output param)
 *			@shm_type: shared memory type, a enum like VGPU_SHM_TYPE_XXX
 *			@priv: kernel driver private data
 * @bs_read_file:	Called when the remote end requests to read a specific file
 *			from local end. Developers need to return the file path of the
 *			file to be read from local end within this function.
 *			@file_type: a enum type of file the remote end wants to read
 *			@priv: kernel driver private data
 *			Return the file name(char *) on success, null pointer on failure.
 * @bs_read_obj:	Called when the remote end requests to read a specific object
 *			from local end. Developers need to memcpy the object on local end that is to
 *			be read into the destination buffer specified in the input parameter.
 *			@dst: the destination buffer where the object to do memcpy
 *			@obj_size: the size of the object on local end to be read  (output param)
 *			@obj_type: the type of object the remote end wants to read
 *			@priv: kernel driver private data
 *			Return 0(VCHANNEL_SUCCESS) on success, negative error code on failure.
 */
struct vchannel_ops {
	void (*setup_vqs)(struct vqueue **vqs, void *priv);
	void (*kick)(uint64_t direction, void *priv);
	void (*write)(struct vqueue_msg *msg, void *priv);
	void (*read)(struct vqueue_msg *msg, uint64_t *ack_data,
		     int8_t *ack_status, void *priv);

	/* for byte stream region operations */
	void (*bs_setup)(uint8_t **buf, uint8_t shm_type, void *priv);
	void *(*bs_read_file)(uint64_t file_type, void *priv);
	int (*bs_read_obj)(void *dst, size_t *obj_size, uint64_t obj_type, void *priv);
};

/**
 * Used in conjunction with the "vchannel_bs_read/VchannelBsRead" function,
 * it represents a request to read a specific object or file type once.
 */
typedef union vchannel_bs_cmd_req {
	uint64_t _cmd;               /* DO NOT set this, used internally. */

	struct {
		uint64_t type : 8;  /* byte stream type, like VGPU_IPC_BS_TYPE */
		uint64_t obj : 16;  /* byte stream subtype */
		uint64_t _first : 1;  /* first cmd, used internally.  */
		uint64_t __reserved : 39;
	};
} vchannel_bs_cmd_req_t;

struct vchannel;

int _vchannel_init(struct vchannel **vchannel, struct vchannel_ops *ops, void *priv, bool is_host);

/**
 * vchannel_uninit - uninitialize a vchannel
 * @vchannel: the vchannel to uninitialize
 * 
 * NOTE: For internal framework use only.
 */
void vchannel_uninit(struct vchannel *vchannel);

/**
 * vchannel_poll - send read requests message on the associated vchannel
 * @vchannel: the vchannel to poll messages
 * @direction: the vqueue direction that needs to be polled
 *
 * Returns true if the number of polled messages > 0, otherwise false.
 * 
 * NOTE: For internal framework use only.
 */
bool vchannel_poll(struct vchannel *vchannel, uint64_t direction);
int _vchannel_read(struct vchannel *vchannel, bool is_host,
		   struct vqueue_msg *read_reqs, uint8_t req_count,
		   struct vqueue_msg *read_acks);
int _vchannel_write(struct vchannel *vchannel, bool is_host,
		    struct vqueue_msg *msgs, uint8_t count);
int _vchannel_link_event(struct vchannel *vchannel, bool is_guest, uint8_t type);
int vchannel_bs_init(struct vchannel *vchannel, vchannel_bs_direction_t direction);
void vchannel_bs_uninit(struct vchannel *vchannel, vchannel_bs_direction_t direction);
int _vchannel_bs_handle_read(struct vchannel *vchannel, uint64_t data,
			     uint64_t *ack, bool is_host);
int _vchannel_bs_read(struct vchannel *vchannel, vchannel_bs_cmd_req_t *bs_req,
		      void *dst, size_t size, bool is_host);

void _ref_vchannel(struct vchannel *vchannel);
void _deref_vchannel(struct vchannel *vchannel);

#ifdef __linux__

#define IS_NATIVE() mtgpu_get_driver_mode() == MTGPU_DRIVER_MODE_NATIVE
#define IS_HOST() mtgpu_get_driver_mode() == MTGPU_DRIVER_MODE_HOST
#define IS_GUEST() mtgpu_get_driver_mode() == MTGPU_DRIVER_MODE_GUEST

/**
 * vchannel_init - initialize a vchannel
 * @vchannel: the double pointer of vchannel
 * @ops: the vchannel callbacks which is implemented by user
 * @priv: user's private data
 *
 * Returns a negative value on error, otherwise 0.
 * 
 * NOTE: For internal framework use only.
 */
static inline int vchannel_init(struct vchannel **vchannel, struct vchannel_ops *ops, void *priv)
{
	return IS_NATIVE() ?
		VCHANNEL_ENOTSUPP : _vchannel_init(vchannel, ops, priv, IS_HOST());
}

/**
 * vchannel_read - send read request messages on the associated vchannel
 * @vchannel: the vchannel to send read messages
 * @read_reqs: an array containing multiple read request messages
 * @req_count: array count
 * @read_acks: an array containing multiple ack messages for read requests
 *
 * Returns a negative value on error, otherwise 0.
 * 
 * NOTE: If the status of any read ack message is erroneous, the function will
 * return the ack_status of the first erroneous ack message as the error code.
 * And If there's an error returned, then the values inside @read_acks are not reliable.
 */
static inline int vchannel_read(struct vchannel *vchannel, struct vqueue_msg *read_reqs,
				uint8_t req_count, struct vqueue_msg *read_acks)
{
	return IS_NATIVE() ?
		VCHANNEL_ENOTSUPP : _vchannel_read(vchannel, IS_HOST(), read_reqs,
						   req_count, read_acks);
}

/**
 * vchannel_write - send write request messages on the associated vchannel
 * @vchannel: the vchannel to send write messages
 * @msgs: an array containing multiple write request messages
 * @count: array count
 *
 * Returns a negative value on error, otherwise 0.
 */
static inline int vchannel_write(struct vchannel *vchannel, struct vqueue_msg *msgs,
				 uint8_t count)
{
	return IS_NATIVE() ?
		VCHANNEL_ENOTSUPP : _vchannel_write(vchannel, IS_HOST(), msgs, count);
}

/**
 * vchannel_read_one - send a read request message on the associated vchannel
 * @vchannel: the vchannel to send read message
 * @read_reqs: a read request messages
 * @data: read ack data (output param), can be set to NULL if it is not needed.
 *
 * Returns a negative value on error, otherwise 0.
 * 
 * NOTE: If there's an error returned, then the value @data are not reliable.
 */
static inline int vchannel_read_one(struct vchannel *vchannel, struct vqueue_msg *read_req,
				    uint64_t *data)
{
	int ret;
	struct vqueue_msg read_ack;

	ret = vchannel_read(vchannel, read_req, 1, &read_ack);

	if (data)
		*data = read_ack.data;

	return ret;
}

/**
 * vchannel_write_one - send a write request message on the associated vchannel
 * @vchannel: the vchannel to send read message
 * @read_reqs: a write request messages
 *
 * Returns a negative value on error, otherwise 0.
 */
static inline int vchannel_write_one(struct vchannel *vchannel, struct vqueue_msg *msg)
{
	return vchannel_write(vchannel, msg, 1);
}

/**
 * vchannel_connect - send a connection request to establish a link and make the vchannel ready
 * @vchannel: the vchannel to send request
 * 
 * Returns a negative value on error, otherwise 0.
 * 
 * NOTE: For internal framework use only.
 */
static inline int vchannel_connect(struct vchannel *vchannel)
{
	return IS_NATIVE() ?
		VCHANNEL_ENOTSUPP : _vchannel_link_event(vchannel, IS_GUEST(), VGPU_IPC_VCHANNEL_CONNECT);
}

/**
 * vchannel_disconnect - send a disconnection request to shutdown the vchannel
 * @vchannel: the vchannel to send request
 * 
 * Returns a negative value on error, otherwise 0.
 * 
 * NOTE: For internal framework use only.
 */
static inline int vchannel_disconnect(struct vchannel *vchannel)
{
	return IS_NATIVE() ?
		VCHANNEL_ENOTSUPP : _vchannel_link_event(vchannel, IS_GUEST(), VGPU_IPC_VCHANNEL_DISCONNECT);
}

/**
 * vchannel_bs_handle_read - handle byte stream read requests on the associated vchannel
 * @vchannel: the vchannel to handle read requests
 * @data: the callback data
 * @ack: the callback ack data pointer
 *
 * Returns a negative value on error, otherwise 0.
 * 
 * NOTE: For internal framework use only.
 */
static inline int vchannel_bs_handle_read(struct vchannel *vchannel, uint64_t data, uint64_t *ack)
{
	return IS_NATIVE() ?
		VCHANNEL_ENOTSUPP : _vchannel_bs_handle_read(vchannel, data, ack, IS_HOST());
}

/**
 * vchannel_bs_read - send byte stream read requests on the associated vchannel
 * @vchannel: the vchannel to send byte stream read requests
 * @req: a byte stream read request
 * @dst: the local object addr to save remote object or the local file path to save remote file.
 * @size: the local object size, only useful when @req->type == VGPU_IPC_BS_TYPE_OBJECT
 *
 * Returns a negative value on error, otherwise 0.
 */
static inline int vchannel_bs_read(struct vchannel *vchannel,
				   vchannel_bs_cmd_req_t *req,
				   void *dst, size_t size)
{
	return IS_NATIVE() ?
		VCHANNEL_ENOTSUPP : _vchannel_bs_read(vchannel, req, dst, size, IS_HOST());
}

static inline void vchannel_ref(struct vchannel *vchannel)
{
	_ref_vchannel(vchannel);
}

static inline void vchannel_deref(struct vchannel *vchannel)
{
	_deref_vchannel(vchannel);
}

#else /* __linux__ */
// For Windows naming style.
typedef struct vchannel             VCHANNEL;
typedef struct vchannel_ops         VCHANNEL_CALLBACKS;
typedef struct vqueue               VQUEUE;
typedef struct vqueue_msg           VQUEUE_MSG;
typedef union vchannel_bs_cmd_req   VCHANNEL_BS_CMD_REQ;
typedef enum vchannel_bs_direction  VCHANNEL_BS_DIRECTION;

static inline int VchannelInit(VCHANNEL** ppVchannel, VCHANNEL_CALLBACKS* pCallbacks,
			       void* pPrivate)
{
	return _vchannel_init(ppVchannel, pCallbacks, pPrivate, false);
}

static inline void VchannelUninit(VCHANNEL* pVchannel)
{
	vchannel_uninit(pVchannel);
}

static inline bool VchannelPoll(VCHANNEL* pVchannel)
{
	return vchannel_poll(pVchannel, VQUEUE_DIRECTION_HOST_TO_GUEST_RW);
}

static inline int VchannelRead(VCHANNEL* pVchannel, VQUEUE_MSG* pReadReqs,
			       uint8_t count, VQUEUE_MSG* pReadAcks)
{
	// Windows is clearly used as a guest, so it should be passed "false".
	return _vchannel_read(pVchannel, false, pReadReqs, count, pReadAcks);
}

static inline int VchannelWrite(VCHANNEL* pVchannel, VQUEUE_MSG* pMsgs, uint8_t count)
{
	return _vchannel_write(pVchannel, false, pMsgs, count);
}

static inline int VchannelReadOne(VCHANNEL* pVchannel, VQUEUE_MSG* pReadReq, uint64_t* pData)
{
	int        ret;
	VQUEUE_MSG readAck;

	ret = VchannelRead(pVchannel, pReadReq, 1, &readAck);

	if (pData)
		*pData = readAck.data;

	return ret;
}

static inline int VchannelWriteOne(VCHANNEL* pVchannel, VQUEUE_MSG* pMsg)
{
	return VchannelWrite(pVchannel, pMsg, 1);
}

static inline int VchannelConnect(VCHANNEL* pVchannel)
{
	return _vchannel_link_event(pVchannel, true, VGPU_IPC_VCHANNEL_CONNECT);
}

static inline int VchannelDisconnect(VCHANNEL* pVchannel)
{
	return _vchannel_link_event(pVchannel, true, VGPU_IPC_VCHANNEL_DISCONNECT);
}

static inline int VchannelBsInit(VCHANNEL* pVchannel, VCHANNEL_BS_DIRECTION direction)
{
	return vchannel_bs_init(pVchannel, direction);
}

static inline void VchannelBsUninit(VCHANNEL* pVchannel, VCHANNEL_BS_DIRECTION direction)
{
	vchannel_bs_uninit(pVchannel, direction);
}

static inline int VchannelBsHandleRead(VCHANNEL* pVchannel, uint64_t data, uint64_t* pAckData)
{
	return _vchannel_bs_handle_read(pVchannel, data, pAckData, false);
}

static inline int VchannelBsRead(VCHANNEL* pVchannel,
				 VCHANNEL_BS_CMD_REQ* pCmdReq,
				 void* pDst, size_t size)
{
	return _vchannel_bs_read(pVchannel, pCmdReq, pDst, size, false);
}

static inline void VchannelReference(VCHANNEL* pVchannel)
{
	_ref_vchannel(pVchannel);
}

static inline void VchannelDereference(VCHANNEL* pVchannel)
{
	_deref_vchannel(pVchannel);
}

#endif /* __linux__ */
#endif /* __VCHANNEL_H__ */
