/*
 * Copyright (c) 2020 STMicroelectronics.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef OPENAMP_VIRTIO_MSG_H
#define OPENAMP_VIRTIO_MSG_H

#include <metal/compiler.h>
#include <metal/utilities.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VIRTIO_MSG_MAX_SIZE		40

/* message type bit field*/
#define VIRTIO_MSG_TYPE_REQ 0
#define VIRTIO_MSG_TYPE_RESP metal_bit(0)

#define VIRTIO_MSG_TYPE_VIRT_MSG 0
#define VIRTIO_MSG_TYPE_BUS_MSG  metal_bit(1)

#define VIRTIO_MSG_FEATURE_SZ_B  32


/**
 * @brief Remote processor states
 */
enum virtio_msg_type {
	VIRTIO_MSG_CONNECT		= 0x01,
	VIRTIO_MSG_DISCONNECT		= 0x02,
	VIRTIO_MSG_DEVICE_INFO		= 0x03,
	VIRTIO_MSG_GET_FEATURES		= 0x04,
	VIRTIO_MSG_SET_FEATURES		= 0x05,
	VIRTIO_MSG_GET_CONFIG		= 0x06,
	VIRTIO_MSG_SET_CONFIG		= 0x07,
	VIRTIO_MSG_GET_CONFIG_GEN	= 0x08,
	VIRTIO_MSG_GET_DEVICE_STATUS	= 0x09,
	VIRTIO_MSG_SET_DEVICE_STATUS	= 0x0A,
	VIRTIO_MSG_GET_VQUEUE		= 0x0B,
	VIRTIO_MSG_SET_VQUEUE		= 0x0C,
	VIRTIO_MSG_RESET_VQUEUE		= 0x0D,
	VIRTIO_MSG_EVENT_CONFIG		= 0x10,
	VIRTIO_MSG_EVENT_AVAIL		= 0x11,
	VIRTIO_MSG_EVENT_USED		= 0x12,
	VIRTIO_MSG_MAX			= VIRTIO_MSG_EVENT_USED,
};

/**
 * @brief VIRTIO_MSG_DEVICE_INFO response message
 *
 * Used by the virtio device to answer to the virtio device info.
 */
METAL_PACKED_BEGIN
struct virtio_msg_get_device_info_resp {
	/* Device virtio version supported */
	uint32_t version;

	/* Device type */
	uint32_t device_id;

	/* Device vendor ID */
	uint32_t vendor_id;
} METAL_PACKED_END;

/**
 * @brief VIRTIO_MSG_GET_FEATURES request message
 *
 * Used by the virtio driver to request the virtio device feature by index.
 */
METAL_PACKED_BEGIN
struct virtio_msg_get_features_req {
	/* Feature 64-bit word index */
	uint32_t index;
} METAL_PACKED_END;

/**
 * @brief VIRTIO_MSG_GET_FEATURES response message
 *
 * Used by the virtio device to answer the virtio device feature by index.
 */
METAL_PACKED_BEGIN
struct virtio_msg_get_features_resp {
	/* Feature 64-bit word index */
	uint32_t index;

	/* Features 8 32-bit word */
	uint32_t features[VIRTIO_MSG_FEATURE_SZ_B / sizeof(uint32_t)];
} METAL_PACKED_END;

/**
 * @brief VIRTIO_MSG_SET_FEATURES request message
 *
 * Used by the virtio driver to set the virtio device feature by index.
 */
METAL_PACKED_BEGIN
struct virtio_msg_set_features_req {
	/* Feature 64-bit word index */
	uint32_t index;

	/* Features 8 32-bit word */
	uint32_t features[VIRTIO_MSG_FEATURE_SZ_B / sizeof(uint32_t)];
} METAL_PACKED_END;

/**
 * @brief VIRTIO_MSG_SET_FEATURES response message
 *
 * Used by the virtio device to answer the virtio device feature by index.
 */
METAL_PACKED_BEGIN
struct virtio_msg_set_features_resp {
	/* Feature 64-bit word index */
	uint32_t index;

	/* Features 8 32-bit word */
	uint32_t features[VIRTIO_MSG_FEATURE_SZ_B / sizeof(uint32_t)];
} METAL_PACKED_END;

/**
 * @brief VIRTIO_MSG_GET_CONFIG response message
 *
 * Used by the virtio driver to get the virtio device configuration.
 */
METAL_PACKED_BEGIN
struct virtio_msg_get_config_req {
	/* 24-bits configuration offset */
	uint8_t offset[3];

	/* Configuration size (1 to 8 bytes) */
	uint8_t size;
} METAL_PACKED_END;

/**
 * @brief VIRTIO_MSG_GET_CONFIG response message
 *
 * Used by the virtio device to answer the virtio device configuration request.
 */
METAL_PACKED_BEGIN
struct virtio_msg_get_config_resp {
	/* 24-bits configuration offset */
	uint8_t offset[3];

	/* Configuration size (1 to 8 bytes) */
	uint8_t size;

	/* Configuration value */
	uint32_t value[8];
} METAL_PACKED_END;

/**
 * @brief VIRTIO_MSG_SET_CONFIG response message
 *
 * Used by the virtio driver to set the virtio device configuration get.
 */
METAL_PACKED_BEGIN
struct virtio_msg_set_config_req {
	/* 24-bits configuration offset */
	uint8_t offset[3];

	/* Configuration size (1 to 8 bytes) */
	uint8_t size;

	/* Configuration value */
	uint32_t value[8];
} METAL_PACKED_END;

/**
 * @brief VIRTIO_MSG_SET_CONFIG response message
 *
 * Used by the virtio device to answer the virtio device configuration set.
 */
METAL_PACKED_BEGIN
struct virtio_msg_set_config_resp {
	/* 24-bits configuration offset */
	uint8_t offset[3];

	/* Configuration size (1 to 8 bytes) */
	uint8_t size;

	/* Configuration value */
	uint32_t value[8];
} METAL_PACKED_END;

/**
 * @brief VIRTIO_MSG_GET_CONFIG_GEN response message
 *
 * Used by the virtio device to answer the virtio device feature by index.
 */
METAL_PACKED_BEGIN
struct virtio_msg_get_config_gen_resp {
	uint32_t generation;
} METAL_PACKED_END;

/**
 * @brief VIRTIO_MSG_GET_DEVICE_STATUS response message
 *
 * Used by the virtio device to answer to the virtio device status get.
 */
METAL_PACKED_BEGIN
struct virtio_msg_get_device_status_resp {
	uint32_t status;
} METAL_PACKED_END;

/**
 * @brief VIRTIO_MSG_SET_DEVICE_STATUS response message
 *
 * Used by the virtio driver to set the virtio device status.
 */
METAL_PACKED_BEGIN
struct virtio_msg_set_device_status_req {
	uint32_t status;
} METAL_PACKED_END;

/**
 * @brief VIRTIO_MSG_GET_VQUEUE request message
 *
 * Used to get the a virtqueue size.
 */
METAL_PACKED_BEGIN
struct virtio_msg_get_vq_req {
	/* virtqueue index */
	uint32_t index;
} METAL_PACKED_END;

/**
 * @brief VIRTIO_MSG_GET_VQUEUE response message
 *
 * Used to provide the virtqueue size.
 */
METAL_PACKED_BEGIN
struct virtio_msg_get_vq_resp {
	/* virtqueue index */
	uint32_t index;

	/* virtqueue max size */
	uint32_t max_size;
} METAL_PACKED_END;

/**
 * @brief VIRTIO_MSG_SET_VQUEUE request message
 *
 * Used to configure the virtqueue.
 */
METAL_PACKED_BEGIN
struct virtio_msg_set_vq_req {
	/* virtqueue index */
	uint32_t index;

	/* virtqueue max size */
	uint32_t unused;

	/* virtqueue max size */
	uint32_t size;

	/* virtqueue descriptor address */
	uint64_t descriptor_addr;

	/* virtqueue driver base address */
	uint64_t driver_addr;

	/* virtqueue device base address */
	uint64_t device_addr;
} METAL_PACKED_END;

/**
 * @brief VIRTIO_MSG_RESET_VQUEUE request message
 *
 * Used to reset the virtqueue
 */
METAL_PACKED_BEGIN
struct virtio_msg_reset_vq_req {
	uint32_t index;
} METAL_PACKED_END;

/**
 * @brief VIRTIO_MSG_EVENT_CONFIG response message
 *
 * Used by the virtio device to answer the virtio device feature by index.
 */
METAL_PACKED_BEGIN
struct virtio_msg_event_config_req {
	/* event configurationstatus */
	uint64_t status;

	/* 24-bits configuration offset */
	uint8_t offset[3];

	/* Configuration size (1 to 16 bytes) */
	uint8_t size;

	/* event configuration value */
	uint8_t value[16];
} METAL_PACKED_END;

/**
 * @brief VIRTIO_MSG_EVENT_AVAIL response message
 *
 * Used to notify on a virtqueue available event.
 */
METAL_PACKED_BEGIN
struct virtio_msg_event_avail_req {
	uint32_t vq_idx;
	uint64_t next_offset;
	uint64_t next_wrap;
} METAL_PACKED_END;

/**
 * @brief VIRTIO_MSG_EVENT_USED response message
 *
 * Used to notify on a virtqueue used event.
 */
METAL_PACKED_BEGIN
struct virtio_msg_event_used_req {
	uint32_t vq_idx;
} METAL_PACKED_END;

/**
 * @brief VIRTIO_MSG_SET_FEATURES response message
 *
 * Used by the virtio device to answer the virtio device feature by index.
 */
METAL_PACKED_BEGIN
struct virtio_msg_virtio_msg {
	uint8_t type;
	uint8_t id;
	uint16_t dev_id;

	union {
		uint8_t payload_u8[36];

		struct virtio_msg_get_device_info_resp get_device_info_resp;
		struct virtio_msg_get_features_req get_features;
		struct virtio_msg_get_features_resp get_features_resp;
		struct virtio_msg_set_features_req set_features;
		struct virtio_msg_set_features_resp set_features_resp;
		struct virtio_msg_get_config_req get_config;
		struct virtio_msg_get_config_resp get_config_resp;
		struct virtio_msg_set_config_req set_config;
		struct virtio_msg_set_config_resp set_config_resp;
		struct virtio_msg_get_config_gen_resp get_config_gen_resp;
		struct virtio_msg_get_device_status_resp get_device_status_resp;
		struct virtio_msg_set_device_status_req set_device_status;
		struct virtio_msg_get_vq_req get_vq;
		struct virtio_msg_get_vq_resp get_vq_resp;
		struct virtio_msg_set_vq_req set_vq;
		struct virtio_msg_reset_vq_req reset_vqueue;
		struct virtio_msg_event_config_req event_config;
		struct virtio_msg_event_avail_req event_avail;
		struct virtio_msg_event_used_req event_used;
	};
} METAL_PACKED_END;


/** @brief A VIRTIO MSG device */
struct virtio_msg_device_cfg {
	/** virtio device id */
	unsigned int dev_id;

	/** virtio vendor id */
	unsigned int vend_id;

	/** virtio versionE */
	unsigned int version;
};


/** @brief A VIRTIO MSG device */
struct virtio_msg_device {
	/** Base virtio device structure */
	struct virtio_device vdev;

	/** VIRTIO_DEV_DRIVER or VIRTIO_DEV_DEVICE */
	unsigned int device_mode;

	/** virtio message bus */
	struct virtio_msg_bus_device *devbus;

	/** device ID on the virtio msg bus */
	unsigned int bus_id;

	/** Custom user data */
	void *user_data;

	/** virtio device node */
	struct metal_list node;

	/** VIRTIO_DEV_DRIVER or VIRTIO_DEV_DEVICE */
	uint32_t  status;

	/** virtio device features bits*/
	uint32_t features[VIRTIO_MSG_FEATURE_SZ_B / sizeof(uint32_t)];
};

int virtio_msg_device_init(struct virtio_msg_device *vmdev, const struct virtio_device_id *dev_id,
			   unsigned int role);

#ifdef __cplusplus
}
#endif

#endif /* OPENAMP_VIRTIO_MSG_H */
