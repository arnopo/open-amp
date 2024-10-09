/*
 * Copyright (c) 2024 STMicroelectronics.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <metal/device.h>
#include <openamp/open_amp.h>
#include <openamp/virtio.h>
#include <openamp/virtio_msg.h>
#include <openamp/virtqueue.h>
#include <stdbool.h>

void virtio_msg_isr(struct virtio_device *vdev);

typedef void (*virtio_msg_vq_callback)(void *);

static int virtio_msg_create_virtqueues(struct virtio_device *vdev, unsigned int flags,
					unsigned int nvqs, const char *names[],
					vq_callback callbacks[], void *callback_args[]);

static void virtio_msg_write_config(struct virtio_device *vdev,
				    uint32_t offset, void *dst, int length)
{
	(void)(vdev);
	(void)(offset);
	(void)(dst);
	(void)length;

	metal_warn("%s not supported\n", __func__);
}

static void virtio_msg_read_config(struct virtio_device *vdev,
				   uint32_t offset, void *dst, int length)
{
	(void)(vdev);
	(void)(offset);
	(void)(dst);
	(void)(length);
}

static void virtio_msg_set_features(struct virtio_device *vdev, uint32_t features)
{
	(void)(vdev);
	(void)(features);

}

static void virtio_msg_reset_device(struct virtio_device *vdev)
{
	(void)(vdev);
}

static void virtio_msg_notify(struct virtqueue *vq)
{
	(void)(vq);
}

static void virtio_msg_dev_set_features(struct virtio_device *vdev, uint32_t features)
{
	struct virtio_msg_device *vmdev =metal_container_of(vdev, struct virtio_msg_device, vdev);

	/* the initialisation should be done when host is not started */
	if (vdev->func->get_status(vdev)) {
	    metal_err( " bad initial state\n");
	    return;
	}

	vmdev->features[0] = features;
}

static uint32_t virtio_msg_dev_get_features(struct virtio_device *vdev)
{
	struct virtio_msg_device *vmdev =metal_container_of(vdev, struct virtio_msg_device, vdev);

	return vmdev->features[0];
}
static uint8_t virtio_msg_dev_get_status(struct virtio_device *vdev)
{
	struct virtio_msg_device *vmdev =metal_container_of(vdev, struct virtio_msg_device, vdev);

	return vmdev->status;
}

const struct virtio_dispatch virtio_dev_msg_dispatch = {
	.create_virtqueues 	= virtio_msg_create_virtqueues,
	.get_status		= virtio_msg_dev_get_status,
	.get_features		= virtio_msg_dev_get_features,
	.set_features		= virtio_msg_dev_set_features,
	.read_config		= virtio_msg_read_config,
	.write_config		= virtio_msg_write_config,
	.notify			= virtio_msg_notify,
};

static inline void virtio_msg_set_status(struct virtio_device *vdev, uint8_t status)
{
	(void)(vdev);
	(void)(status);
}

static uint8_t virtio_msg_get_status(struct virtio_device *vdev)
{
	(void)(vdev);

	return 0;
}

const struct virtio_dispatch virtio_drv_msg_dispatch = {
	.create_virtqueues = virtio_msg_create_virtqueues,
	.get_status = virtio_msg_get_status,
	.set_status = virtio_msg_set_status,
	.set_features = virtio_msg_set_features,
	.read_config = virtio_msg_read_config,
	.write_config = virtio_msg_write_config,
	.reset_device = virtio_msg_reset_device,
	.notify = virtio_msg_notify,
};


int virtio_msg_device_init(struct virtio_msg_device *vmdev, const struct virtio_device_id *dev_id,
			   unsigned int role)
{
	vmdev->vdev.id = *dev_id;
	vmdev->vdev.role = role;

	if (VIRTIO_ROLE_IS_DEVICE(&vmdev->vdev))
		vmdev->vdev.func = &virtio_dev_msg_dispatch;
	else if (VIRTIO_ROLE_IS_DRIVER(&vmdev->vdev))
		vmdev->vdev.func = &virtio_drv_msg_dispatch;
	else
	    metal_err( " bad configuration\n");



	return 0;
}

/* Register preallocated virtqueues */
void virtio_msg_register_device(struct virtio_device *vdev, int vq_num, struct virtqueue **vqs)
{
	(void)(vdev);
	(void)(vq_num);
	(void)(vqs);
}

struct virtqueue *virtio_msg_setup_virtqueue(struct virtio_device *vdev,
					     unsigned int idx,
					     struct virtqueue *vq,
					     void (*cb)(void *),
					     void *cb_arg,
					     const char *vq_name)
{
	(void)(vdev);
	(void)(idx);
	(void)(vq);
	(void)(cb);
	(void)(cb_arg);
	(void)(vq_name);

	return NULL;
}

void virtio_msg_isr(struct virtio_device *vdev)
{
	(void)(vdev);

}

static int virtio_msg_create_virtqueues(struct virtio_device *vdev, unsigned int flags,
					unsigned int nvqs, const char *names[],
					vq_callback callbacks[], void *callback_args[])
{
	(void)(vdev);
	(void)(flags);
	(void)(nvqs);
 	(void)(names);
	(void)(callbacks);
  	(void)(callback_args);

	return 0;
}
