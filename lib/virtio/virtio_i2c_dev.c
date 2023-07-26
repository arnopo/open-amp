/*
 * Copyright (c) 2023, STMicroelectronics
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <openamp/virtio_i2c_dev.h>
#include <metal/utilities.h>

bool virtio_i2c_ready(struct virtio_i2c_dev *dev)
{
	return dev->enabled;
}

int virtio_i2c_handle_avail(struct virtio_i2c_dev *dev)
{
	/** we use the first virtqueue as virtio i2c only need one */
	struct virtqueue *vq = dev->vdev->vrings_info[0].vq;
	void *descs[VIRTIO_I2C_BUFFERS];
	uint16_t idxs[VIRTIO_I2C_BUFFERS];
	uint32_t lens[VIRTIO_I2C_BUFFERS];
	struct virtio_i2c_out_hdr *header;
	struct virtio_i2c_msg msg;
	uint8_t *returnCode;
	int used = 0, ret;
	bool read;

	if (!vq)
		return -1;

	/** We use all available buffers */
	while (virtqueue_get_desc_size(vq)) {

		/** get the first buffer of the chain */
		descs[0] = virtqueue_get_available_buffer(vq, &idxs[0], &lens[0]);

		/** get the rest of the chain */
		for (int i = 1; i < VIRTIO_I2C_BUFFERS; i++) {
			idxs[i] = idxs[i-1];
			descs[i] = virtqueue_get_next_buffer(vq, &idxs[i], &lens[i]);
		}

		/** we need at least 2 buffers */
		if (!descs[VIRTIO_I2C_HEADER_BUF] ||
		    !descs[VIRTIO_I2C_DATA_BUF])
			return -EINVAL;

		/** we may not have the data buffer if it's a zero length requests,
		 * so in that case the second buffer is the return buffer
		 */
		if (!descs[VIRTIO_I2C_RETURN_BUF]) {
			descs[VIRTIO_I2C_RETURN_BUF] = descs[VIRTIO_I2C_DATA_BUF];
			idxs[VIRTIO_I2C_RETURN_BUF] = idxs[VIRTIO_I2C_DATA_BUF];
			lens[VIRTIO_I2C_RETURN_BUF] = lens[VIRTIO_I2C_DATA_BUF];
			descs[VIRTIO_I2C_DATA_BUF] = NULL;
		}

		/** check if i2c header is valid */
		if (virtqueue_buffer_writable(vq, idxs[VIRTIO_I2C_HEADER_BUF]) ||
		    lens[VIRTIO_I2C_HEADER_BUF] != sizeof(struct virtio_i2c_out_hdr))
			return -EINVAL;

		/** check if the return buf is valid */
		if (!descs[VIRTIO_I2C_RETURN_BUF] ||
		    !virtqueue_buffer_writable(vq, idxs[VIRTIO_I2C_RETURN_BUF]))
			return -EINVAL;

		/** We extract the data from the buffers */
		header = (struct virtio_i2c_out_hdr *)descs[VIRTIO_I2C_HEADER_BUF];
		returnCode = (uint8_t *)descs[VIRTIO_I2C_RETURN_BUF];

		/** check if this is a read or write request */
		read = header->flags & metal_bit(VIRTIO_I2C_MSG_READ);

		/** if we have a data buffer, then it should be write only for read request
		 * and read only for write request
		 */
		if (descs[VIRTIO_I2C_DATA_BUF] &&
		    virtqueue_buffer_writable(vq, idxs[VIRTIO_I2C_DATA_BUF]) != read)
			return -EIO;

		msg.addr = header->addr >> 1;
		msg.buf = descs[VIRTIO_I2C_DATA_BUF];
		msg.len = descs[VIRTIO_I2C_DATA_BUF] ? lens[VIRTIO_I2C_DATA_BUF] : 0;
		msg.flags = read ? VIRTIO_I2C_MSG_READ : VIRTIO_I2C_MSG_WRITE;


		/** send request on the bus */
		ret = dev->config.cb(dev->config.cb_data, &msg);

		/** 1 is fail, 0 is success */
		*returnCode = ret ? 1 : 0;

		virtqueue_add_consumed_buffer(vq, idxs[VIRTIO_I2C_HEADER_BUF],
					      (read && !ret) ? msg.len + 1 : 1);
		used++;
	}

	if (used)
		virtqueue_kick(vq);

	return used;
}

int virtio_i2c_configure(struct virtio_i2c_dev *i2c_dev, void (*vq_cb)(struct virtqueue *vq),
			 struct virtio_i2c_config *config)
{
	struct virtio_device *vdev = i2c_dev->vdev;

	if(!i2c_dev ||!vdev || !config)
		return -EINVAL;

	if (!config->cb)
		return -EINVAL;

	vdev->priv = i2c_dev;
	vdev->vrings_info = &i2c_dev->vring;
	vdev->vrings_info->vq = &i2c_dev->vq;
	vdev->vrings_info->vq->callback = vq_cb;
	vdev->vrings_info->vq->priv = i2c_dev;

	vdev->vrings_num = 1;

	i2c_dev->vdev = vdev;
	memcpy(&i2c_dev->config, config, sizeof(*config));

	vdev->func->set_features(vdev, metal_bit(VIRTIO_I2C_F_ZERO_LENGTH_REQUEST));

	i2c_dev->enabled = true;

	return 0;
}
