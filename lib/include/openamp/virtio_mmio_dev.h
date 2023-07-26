/*
 * Copyright (c) 2023, STMicroelectronics
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef OPENAMP_VIRTIO_MMIO_DEV_H
#define OPENAMP_VIRTIO_MMIO_DEV_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <metal/utilities.h>
#include <metal/device.h>
#include <openamp/virtio_mmio.h>

/**
 * @brief Define an Empty MMIO register table with only the strict necessary
 * for a driver to recognise the device
 */
#define EMPTY_MMIO_TABLE {					\
	.magic = VIRTIO_MMIO_MAGIC_VALUE_STRING,		\
	.version = 2,						\
	.status = VIRTIO_CONFIG_STATUS_NOT_READY,		\
}

/**
 * @brief MMIO Device Registers: 256 bytes in practice, more if the configuration space is used.
 * This is a trimmed down version with only the essential values needed to be detected correctly.
 */
struct mmio_table {

	/* 0x00 R should be 0x74726976 */
	uint32_t magic;

	/* 0x04 R */
	uint32_t version;

	/* padding */
	uint32_t padding[26];

	/* 0x70 RW Writing non-zero values to this register sets the status flags,
	 * indicating the driver progress.
	 * Writing zero (0x0) to this register triggers a device reset.
	 */
	uint32_t status;
};

/**
 * @brief This is called when the other side should be notifyed
 *
 * @param dev The device that need to notify the other side
 */
typedef void (*virtio_notify)(struct virtio_device *dev);

/**
 * @brief VirtIO mmio dev instance, should be init with mmio_dev_init,
 * then the VirtIO driver should set it's data using mmio_dev_set_device_data
 *
 * @param vdev VirtIO device instance
 * @param vring_size Number of descriptors per ring
 * @param vqs Array of virtqueues
 * @param io Metal IO Region used to access the MMIO registers
 * @param notify Called when an interrupt should be sent to the other side
 * @param device_features The features supported by this device
 * @param driver_features The features supported by the driver from the other side
 */
struct virtio_mmio_dev {
	struct virtio_device		vdev;
	int				vring_size;
	struct virtqueue		*vqs;
	struct metal_io_region		*io;
	virtio_notify			notify;
	uint64_t			device_features;
	uint64_t			driver_features;
};

/**
 * @brief This should be called to initialize a virtio mmio device,
 * the configure function should be called next by the device driver
 *
 * @param dev The device to initialize
 * @param io The memory region in wich the device should operate
 * @param callback The callback that will be called when the other side should be notifyed
 */
void mmio_dev_init(struct virtio_mmio_dev *dev, struct metal_io_region *io, virtio_notify callback);

/**
 * @brief Should be called by the app when it receive an interrupt for the mmio device
 *
 * @param dev The virtio mmio device
 */
void mmio_dev_interrupt(struct virtio_mmio_dev *dev);

/**
 * @brief Utils function to read a register of the mmio registers
 *
 * @param dev The virtio mmio device
 * @param address The address of the register
 *
 * @returns register value
 */
static inline uint32_t read_reg(struct virtio_mmio_dev *dev, uint32_t address)
{
	return metal_io_read32(dev->io, address);
}

/**
 * @brief Utils function to write in a register of the mmio registers
 *
 * @param dev The virtio mmio device
 * @param address The address of the register
 * @param value The value to write
 */
static inline void write_reg(struct virtio_mmio_dev *dev, uint32_t address, uint32_t value)
{
	metal_io_write32(dev->io, address, value);
}

/**
 * @brief This function will set the callbacks of the virtqueues, the device driver should wait
 * until the virtio device is fully initialized before calling this
 *
 * @param vdev The virtio device
 * @param flags not used
 * @param nvqs The number of virtiqueue configure
 * @param names Array of names to give to each virtqueue
 * @param callbacks Array of callbacks to give to each virtqueue
 *
 * @returns 0 if successful
 */
static int mmio_dev_create_virtqueues(struct virtio_device *vdev, unsigned int flags,
				      unsigned int nvqs, const char **names,
				      vq_callback * callbacks);

/**
 * @brief This function will reset the configuration of the device and free all memory
 *
 * @param vdev The virtio device
 */
static void mmio_dev_delete_virtqueues(struct virtio_device *vdev);

/**
 * @brief This should be called by the device driver to set it's features and device type
 * Once this is called the virtio mmio status is no longer: not ready
 *
 * @param dev The virtio mmio device
 * @param device_features The features supported by the driver
 * @param device_type The device type of the driver
 * @param nvqs The max number of virtqueue allowed by the device
 *
 * @returns 0 if successful
 */
static int mmio_dev_configure_device(struct virtio_device *vdev, uint64_t device_features,
				     uint32_t device_type, int nvqs, int vring_size);

/**
 * @brief This function returns the status of the virtio device
 *
 * @param vdev The virtio device
 *
 * @returns The status of the device
 */
static uint8_t mmio_dev_get_status(struct virtio_device *vdev);

/**
 * @brief This function set the status of the virtio device
 *
 * @param vdev The virtio device
 * @param status The status to set on the virtio device
 */
static void mmio_dev_set_status(struct virtio_device *vdev, uint8_t status);

/**
 * @brief This function returns the features supported by the virtio device
 *
 * @param vdev The virtio device
 *
 * @returns The features of the device
 */
static uint64_t mmio_dev_get_features(struct virtio_device *vdev);

/**
 * @brief This function set the features supported by the virtio device
 *
 * @param vdev The virtio device
 * @param feature The features to set on the virtio device
 */
static void mmio_dev_set_features(struct virtio_device *vdev, uint32_t feature);

/**
 * @brief This function read length byte of the configuration of the virtio device in the dst buffer
 *
 * @param vdev The virtio device
 * @param offset The offset in the config memory
 * @param dst The destination buffer to write into
 * @param length The length of config to copy in the dst buffer
 */
static void mmio_dev_read_config(struct virtio_device *vdev, uint32_t offset, void *dst,
				 int length);

/**
 * @brief This function write length byte of src buffer in the configuration of the virtio device
 *
 * @param vdev The virtio device
 * @param offset The offset in the config memory
 * @param src The source buffer to read from
 * @param length The length of config to write from the src buffer
 */
static void mmio_dev_write_config(struct virtio_device *vdev, uint32_t offset, void *src,
				  int length);

/**
 * @brief This function will trigger a reset of the device
 *
 * @param vdev The virtio device
 */
static void mmio_dev_reset_device(struct virtio_device *vdev);

/**
 * @brief This function is not yet used and may not be needed
 *
 * @param vdev The virtio device
 */
static void mmio_dev_notify(struct virtqueue *vq);

/**
 * @brief The virtio api function that this virtio device supports
 */
static struct virtio_dispatch virtio_ops = {

	.create_virtqueues = mmio_dev_create_virtqueues,
	.delete_virtqueues = mmio_dev_delete_virtqueues,

	.configure_device = mmio_dev_configure_device,

	.get_status = mmio_dev_get_status,
	.set_status = mmio_dev_set_status,

	.get_features = mmio_dev_get_features,
	.set_features = mmio_dev_set_features,

	.read_config = mmio_dev_read_config,
	.write_config = mmio_dev_write_config,

	.reset_device = mmio_dev_reset_device,
};

#endif
