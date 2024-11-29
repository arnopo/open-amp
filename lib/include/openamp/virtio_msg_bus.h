/*
 * Copyright (c) 2020 STMicroelectronics.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef OPENAMP_VIRTIO_MSG_BUS_H
#define OPENAMP_VIRTIO_MSG_BUS_H

#include <metal/atomic.h>

#ifdef __cplusplus
extern "C" {
#endif

struct virtio_msg_bus_device;

struct virtio_msg_bus_cfg {

	metal_phys_addr_t msg_paddr;
	uintptr_t msg_vaddr;
	uint32_t msg_size;
	uint32_t rx_elts;
	uint32_t tx_elts;

	metal_phys_addr_t shm_paddr;
	uintptr_t shm_vaddr;
	size_t shm_size;

	/** VIRTIO_DEV_DRIVER or VIRTIO_DEV_DEVICE */
	unsigned int role;

	/** Notify the other side that a virtio msg has been sent */
	int (*notify)(struct virtio_msg_bus_device *dev);
};

struct virtio_msg_bus_device {

	/* virtio msg device */
	struct virtio_msg_device vmdev;

	/* bus identifier */
	uint32_t bus_id;

	/* private data to retrieve the context */
	void *priv;

	const struct virtio_msg_bus_cfg *cfg;

	/* shared memory queues for virtio message exchange */
	struct amp_static_queues sq;

	/** shared memory metal_io_region for message exchange*/
	struct metal_io_region msg_io;
	/** shared memory metal_io_region for vrgins and buffers*/
	struct metal_io_region shm_io;

	/** List of virtio device */
	struct metal_list vdevs;

	unsigned int state;

	struct virtqueue	*vqs;

	unsigned int		num_vqs;
};

int virtio_msg_bus_init(struct virtio_msg_bus_device *devbus,
			const struct virtio_msg_bus_cfg *bus_cfg, void *priv);
int virtio_msg_bus_connect(struct virtio_msg_bus_device *devbus, unsigned int timeout);
int virtio_msg_bus_disconnect(struct virtio_msg_bus_device *devbus, unsigned int timeout);

int virtio_msg_bus_receive(struct virtio_msg_bus_device *devbus);
int virtio_msg_bus_register_vdev(struct virtio_msg_bus_device *devbus,
				 struct virtio_msg_device *vmdev,
				 const struct virtio_device_id *dev_id,
				 unsigned int role);
#ifdef __cplusplus
}
#endif

#endif /* OPENAMP_VIRTIO_MSG_BUS_H */
