/*
 * Copyright (c) 2023, STMicroelectronics
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef OPENAMP_VIRTIO_I2C_DEV_H
#define OPENAMP_VIRTIO_I2C_DEV_H

#include <openamp/virtio.h>

/** Number of buffer per request for I2C virtio */
#define VIRTIO_I2C_BUFFERS	3

/** Indexes of I2C virtio buffers */
#define VIRTIO_I2C_HEADER_BUF	0
#define VIRTIO_I2C_DATA_BUF	1
#define VIRTIO_I2C_RETURN_BUF	2

/** Support for Zero length request with only 2 buffers */
#define VIRTIO_I2C_F_ZERO_LENGTH_REQUEST	0

/** Write message to I2C bus. */
#define VIRTIO_I2C_MSG_WRITE	0
/** Read message from I2C bus. */
#define VIRTIO_I2C_MSG_READ	1
/** Send STOP after this message. */
#define VIRTIO_I2C_MSG_STOP	2

/**
 * @brief This defines one I2C message to send on the I2C bus.
 */
struct virtio_i2c_msg {
	uint8_t		*buf;
	uint32_t	len;
	uint16_t	addr;
	uint8_t		flags;
};

/**
 * @brief This defines the header of an I2C request
 */
struct virtio_i2c_out_hdr {
	uint16_t addr;
	uint16_t padding;	/** Not used */
	uint32_t flags;
};

/**
 * @brief This is called when an I2C transaction is ready to be sent on the bus
 *
 * @param callback_data The data that was given when the device was configured
 * @param msg The i2c msg that should be written or red on the bus
 */
typedef int (*i2c_callback)(void *callback_data, struct virtio_i2c_msg *msg);

/** @brief This represent a virtio i2c device instance and should be configured before use */
struct virtio_i2c_config {
	/**  Called when an I2C transaction should be performed */
	i2c_callback		cb;

	/**  Data given in the callback */
	void			*cb_data;

	/** The device Identifier on the virtio transport layer */
	unsigned int bus_id;
};

/**
 * @brief This represent a virtio i2c device instance and should be configured before use
 *
 * @param vdev
 * @param callback Called when an I2C transaction should be performed
 * @param callback_data Data given in the callback
 */
struct virtio_i2c_dev {
	/** The VirtIO device driver */
	struct virtio_device	*vdev;

	/**  Associated configuration*/
	struct virtio_i2c_config config;

	/** virtio i2c virtqueue */
	struct virtio_vring_info vring;
	struct virtqueue vq;

	/** i2c device bus state */
	bool enabled;
};

/**
 * @brief This is used to configure a new virtio i2c device instance,
 * it should be called before using the instance
 *
 * @param dev The instance to configure
 * @param virtio_mmio_dev An instance of a virtio mmio device
 * @param vq_cb optional function to call when an I2C transaction is received
 * @param config virtio i2c device configuration structure
 *
 * @returns 0 if successful
 */
int virtio_i2c_configure(struct virtio_i2c_dev *dev, void (*vq_cb)(struct virtqueue *vq),
			 struct virtio_i2c_config *config);


/**
 * @brief Get the status of the virtio device
 *
 * @param dev The device instance
 *
 * @returns true if ready
 */
bool virtio_i2c_ready(struct virtio_i2c_dev *dev);

/**
 * @brief This is used to handle available buffers in the virtqueue
 *
 * @param dev The device to handle buffers for
 *
 * @returns number of buffers used
 */
int virtio_i2c_handle_avail(struct virtio_i2c_dev *dev);

#endif
