/*
 * Copyright (c) 2024 STMicroelectronics.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <metal/device.h>
#include <metal/log.h>
#include <openamp/open_amp.h>
#include <openamp/virtio.h>
#include <openamp/virtio_msg.h>
#include <openamp/virtio_msg_bus.h>

#define VIRTIO_MSG_VIRTQUEUE_MAX_SIZE  0x10

#ifndef VIRTIO_MSG_QUEUE_DRV_SIZE
	#define VIRTIO_MSG_QUEUE_DRV_SIZE  4
#endif
#ifndef VIRTIO_MSG_QUEUE_DEV_SIZE
	#define VIRTIO_MSG_QUEUE_DEV_SIZE  4
#endif

#define VIRTIO_MSG_QUEUE_CFG {					\
		.drv_elt_size = VIRTIO_MSG_MAX_SIZE,		\
		.drv_num_elts = VIRTIO_MSG_QUEUE_DRV_SIZE,	\
		.dev_elt_size = VIRTIO_MSG_MAX_SIZE,		\
		.dev_num_elts = VIRTIO_MSG_QUEUE_DRV_SIZE,	\
	}

static int virtio_msg_bus_send(struct virtio_msg_bus_device *devbus,
			       struct virtio_msg_virtio_msg *msg)
{
	int ret;

	if (!devbus || !devbus->cfg)
		return -EINVAL;

	metal_dbg("v-msg TX: T %#x, ID %#x, D-ID %#x\n", msg->type, msg->id, msg->dev_id);

	ret = amp_static_queue_send(&devbus->sq, msg, (int)sizeof(*msg));
	if (ret < 0)
		return ret;

	if (!devbus->cfg->notify)
		return 0;

	return devbus->cfg->notify(devbus);
}

static int virtio_msg_get_dev_feats(struct virtio_msg_device *vmdev,
				    struct virtio_msg_virtio_msg *msg)
{
	uint32_t index = msg->get_features.index;

	if(!vmdev->vdev.func || !vmdev->vdev.func->get_features)
		return -EINVAL;

	if (index) {
		metal_err("Non null index not yet supported\n");
		return -EINVAL;
	}

	msg->get_features_resp.index = index;

	msg->get_features_resp.features[0] = vmdev->vdev.func->get_features(&vmdev->vdev);

	return 0;
}

static int virtio_msg_set_dev_feats(struct virtio_msg_device *vmdev,
				    struct virtio_msg_virtio_msg *msg)
{
	uint32_t index = msg->get_features.index;

	if(!vmdev->vdev.func->set_features)
		return -EINVAL;

	if (index) {
		metal_err("Non null index not yet supported\n");
		return -EINVAL;
	}

	msg->get_features_resp.index = index;

	vmdev->vdev.func->set_features(&vmdev->vdev, msg->get_features_resp.features[0]);

	return 0;
}

static void virtio_msg_bus_vq_notify(struct virtqueue *vq)
{
	struct virtio_device *vdev = vq->vq_dev;
	struct virtio_msg_device *vmdev = metal_container_of(vdev, struct virtio_msg_device, vdev);
	struct virtio_msg_virtio_msg msg;

	msg.type = VIRTIO_MSG_TYPE_REQ;
	msg.id = VIRTIO_MSG_EVENT_USED;
	msg.dev_id = vmdev->bus_id;
	msg.event_used.vq_idx = vq->vq_queue_index;

	virtio_msg_bus_send(vmdev->devbus, &msg);
}

static int  virtio_msg_set_virtqueue(struct virtio_msg_device *vmdev,
				     struct virtio_msg_bus_device *devbus,
				     struct virtio_msg_virtio_msg *msg)
{
	struct virtio_msg_set_vq_req  *vq_cfg = &msg->set_vq;
	struct vring_alloc_info *alloc_info;
	struct virtio_vring_info *vinfo;
	struct virtqueue *vq;
	int ret;

	if (vq_cfg->index > vmdev->vdev.vrings_num) {
		metal_err("invalid vring index\n");
		return -EINVAL;
	}

	vinfo = &vmdev->vdev.vrings_info[vq_cfg->index];
	alloc_info = &vinfo->info;
	vq = vinfo->vq;

	/** If the host set Queue Ready then we can read the virtqueue */
	alloc_info->num_descs = vq_cfg->size;

	alloc_info->vaddr = (void *)(uintptr_t)vq_cfg->driver_addr;

	/** TODO: the alignment should be set in a config instead of being hardcoded */
	alloc_info->align = 4096;

	/** create an empty virtqueue that will be completed in create_virtqueues */
	ret = virtqueue_create(&vmdev->vdev, vq_cfg->index, "vq", alloc_info,
			       vq->callback, virtio_msg_bus_vq_notify, vq);
	if (ret)
		return ret;

	vq->shm_io = &devbus->shm_io;
	vinfo->vq = vq;

	/** Use the vring addresses provided in case they were aligned differently */
	vq->vq_ring.desc = (struct vring_desc *)(uintptr_t)vq_cfg->descriptor_addr;
	vq->vq_ring.avail = (struct vring_avail *)(uintptr_t)vq_cfg->driver_addr;
	vq->vq_ring.used = (struct vring_used *)(uintptr_t)vq_cfg->device_addr;

	return 0;
}

static int virtio_msg_bus_ev_available(struct virtio_msg_device *vmdev,
				       struct virtio_msg_event_avail_req *req)
{
	struct virtqueue *vq;

	vq = vmdev->vdev.vrings_info[req->vq_idx].vq;

	/* get message from vq */
	virtqueue_notification(vq);

	return 0;
}

int virtio_msg_bus_receive(struct virtio_msg_bus_device *devbus)
{
	struct virtio_msg_virtio_msg msg;
	struct virtio_msg_device *vmdev;
	struct metal_list *node;
	int ret;

	ret = amp_static_queue_receive(&devbus->sq, &msg, sizeof(msg));
	if (ret) {
		if (ret != -ENOMEM)
			metal_dbg("error when read message\n");
		return ret;
	}

	printf("v-msg RX: T %#x, ID %#x, D-ID %#x\r\n", msg.type, msg.id, msg.dev_id);

	/* Find the destination device */
	metal_list_for_each(&devbus->vdevs, node) {
		vmdev = metal_container_of(node, struct virtio_msg_device, node);
		if (vmdev->bus_id == msg.dev_id)
			break;
		vmdev = NULL;
	}

	if (!vmdev) {
		metal_err("unknown device id (%d)\n", msg.dev_id);
		/* should we send an error message ?*/
		return  0;
	}

	switch (msg.id) {
	case VIRTIO_MSG_DEVICE_INFO:
		msg.get_device_info_resp.device_id = vmdev->vdev.id.device;
		msg.get_device_info_resp.vendor_id = vmdev->vdev.id.vendor;
		break;
	case VIRTIO_MSG_GET_FEATURES:
		ret = virtio_msg_get_dev_feats(vmdev, &msg);
		if (ret)
			return ret;
	break;
	case VIRTIO_MSG_SET_FEATURES:
		return virtio_msg_set_dev_feats(vmdev, &msg);
	break;
	case VIRTIO_MSG_GET_VQUEUE:
		msg.get_vq_resp.max_size = VIRTIO_MSG_VIRTQUEUE_MAX_SIZE;
	break;
	case VIRTIO_MSG_SET_VQUEUE:
		ret = virtio_msg_set_virtqueue(vmdev, devbus, &msg);
		return ret;
	break;
	case VIRTIO_MSG_SET_DEVICE_STATUS:
		if (VIRTIO_ROLE_IS_DEVICE(&vmdev->vdev))
			vmdev->status = msg.set_device_status.status;
		return 0;
	case VIRTIO_MSG_GET_DEVICE_STATUS:

		if (VIRTIO_ROLE_IS_DEVICE(&vmdev->vdev))
			msg.get_device_status_resp.status = vmdev->status;
		break;
	case VIRTIO_MSG_EVENT_AVAIL:
		virtio_msg_bus_ev_available(vmdev, &msg.event_avail);
		return 0;
	default:
		metal_err("Ignore send request (%d)\n", msg.type);
		return -EINVAL;
	}

	return  virtio_msg_bus_send(devbus, &msg);
}

int virtio_msg_bus_register_vdev(struct virtio_msg_bus_device *devbus,
				 struct virtio_msg_device *vmdev,
				 const struct virtio_device_id *dev_id,
				 unsigned int role)
{
	if(!vmdev || !devbus)
		return -EINVAL;

	metal_dbg("register vdev ID %#x, BUS_ID %#x\n", vmdev->vdev.id, vmdev->bus_id);

	virtio_msg_device_init(vmdev, dev_id, role);

	metal_list_add_tail(&devbus->vdevs, &vmdev->node);
	vmdev->devbus = devbus;

	return 0;
}

int virtio_msg_bus_disconnect(struct virtio_msg_bus_device *devbus, unsigned int timeout)
{
	struct virtio_msg_virtio_msg msg;
	(void)(timeout);

	msg.type = VIRTIO_MSG_TYPE_REQ | VIRTIO_MSG_TYPE_BUS_MSG;
	msg.id = VIRTIO_MSG_DISCONNECT;

	return virtio_msg_bus_send(devbus, &msg);
}

int virtio_msg_bus_connect(struct virtio_msg_bus_device *devbus, unsigned int timeout)
{
	(void)(timeout);
	int ret;

	ret = amp_static_queue_connect(&devbus->msg_io, &devbus->sq);
	if (ret)
		return ret;

	/* Notify the remote side that we are ready to communicate */
	if (devbus->cfg->role == VIRTIO_DEV_DEVICE)
		return devbus->cfg->notify(devbus);

	return 0;
}

int virtio_msg_bus_init(struct virtio_msg_bus_device *devbus,
			const struct virtio_msg_bus_cfg *bus_cfg, void *priv)
{
	const struct amp_static_queues_cfg q_cfg = VIRTIO_MSG_QUEUE_CFG;
	devbus->cfg = bus_cfg;
	devbus->priv = priv;
	int ret;
	if (!devbus || !bus_cfg || !bus_cfg->shm_paddr || !bus_cfg->shm_vaddr)
		return -EINVAL;

	metal_io_init(&devbus->msg_io, (void *)bus_cfg->msg_vaddr, &bus_cfg->msg_paddr,
		      bus_cfg->msg_size, -1, 0, NULL);

	metal_io_init(&devbus->shm_io, (void *)bus_cfg->shm_vaddr, &bus_cfg->shm_paddr,
		      bus_cfg->shm_size, -1, 0, NULL);

	ret = amp_static_queue_dev_init(&devbus->msg_io, &q_cfg, &devbus->sq);
	if (ret)
		return ret;

	metal_list_init(&devbus->vdevs);

	return 0;
}

int virtio_msg_bus_deinit(struct virtio_msg_bus_device *devbus,
			const struct virtio_msg_bus_cfg *bus_cfg, void *priv)
{
	const struct amp_static_queues_cfg q_cfg = VIRTIO_MSG_QUEUE_CFG;
	devbus->cfg = bus_cfg;
	devbus->priv = priv;
	int ret;
	if (!devbus || !bus_cfg || !bus_cfg->shm_paddr || !bus_cfg->shm_vaddr)
		return -EINVAL;


	ret = amp_static_queue_dev_init(&devbus->msg_io, &q_cfg, &devbus->sq);
	if (ret)
		return ret;

	metal_list_init(&devbus->vdevs);

	return 0;
}
