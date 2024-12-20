/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024, Linaro LTD
 * Copyright (c) 2024, STMicroelectronics
 *
 */

#include <openamp/amp_static_queue.h>
#include <metal/io.h>

#define MAGIC_Q_DEF 0x1A2B3C4D
#define ALIGN_64(num)    (((num) + 7) & ~(7))

#define VIRTIO_MSG_QUEUE_DEF_INIT {			\
		.magic = MAGIC_Q_DEF,			\
		.version = 1,				\
	}

int amp_static_queue_send(struct amp_static_queues *sq, void *msg, size_t msg_len)
{
	struct amp_static_queue *rx = &sq->rx;
	struct amp_static_queue *tx = &sq->tx;
	struct amp_static_queue_head qh;
	unsigned long offset;
	uint8_t tail;
	uint8_t next;
	size_t len;

	len = metal_io_block_read(sq->io, rx->head, &qh, sizeof(qh));
	if (len != sizeof(qh))
		return -EINVAL;
	tail = qh.r_idx;

	len = metal_io_block_read(sq->io, tx->head, &qh, sizeof(qh));
	if (len != sizeof(qh))
		return -EINVAL;

	/* compute the head pointer */
	next = qh.w_idx + 1;
	if (next == tx->num_elts)
		next = 0;

	/* If the queue is full, bail out */
	if (next == tail)
		return -EBUSY;

	offset = tx->data + qh.w_idx * tx->elt_size;

	len = metal_io_block_write(sq->io, offset, msg, msg_len);
	if (len != msg_len)
		return -EINVAL;

	qh.w_idx = next;
	len = metal_io_block_write(sq->io, tx->head, &qh, sizeof(qh));
	if (len != sizeof(qh))
		return -EINVAL;

	return 0;
}

int amp_static_queue_receive(struct amp_static_queues *sq, void *msg, size_t msg_len)
{
	struct amp_static_queue *rx = &sq->rx;
	struct amp_static_queue *tx = &sq->tx;
	struct amp_static_queue_head qh;
	unsigned long offset;
	uint8_t tail;
	uint8_t head;
	size_t len;

	len = metal_io_block_read(sq->io, rx->head, &qh, sizeof(qh));
	if (len != sizeof(qh))
		return -EINVAL;

	tail = qh.w_idx;

	len = metal_io_block_read(sq->io, tx->head, &qh, sizeof(qh));
	if (len != sizeof(qh))
		return -EINVAL;

	head = qh.r_idx;

	/* If the queue is empty, bail out */
	if (head == tail)
		return -ENOMEM;

	offset = rx->data + head * tx->elt_size;
	len = metal_io_block_read(sq->io, offset, msg, msg_len);
	if (len != msg_len)
		return -EINVAL;

	if (++head == rx->num_elts)
		head = 0;

	qh.r_idx = head;
	len = metal_io_block_write(sq->io, tx->head, &qh, sizeof(qh));
	if (len != sizeof(qh))
		return -EINVAL;

	return 0;
}

int amp_static_queue_connect(struct metal_io_region *io, struct amp_static_queues *sq)
{
	struct amp_static_queue_head queue_head = {.status = AMP_STATIC_QUEUE_READY};
	size_t len;

	/* Initialise the amp_static_queue_def in shared memory */
	len = metal_io_block_write(io, sq->tx.head, &queue_head, sizeof(queue_head));
	if (len != sizeof(queue_head))
		return -EINVAL;

	return 0;
}

int amp_static_queue_dev_init(struct metal_io_region *io, const struct amp_static_queues_cfg *cfg,
			      struct amp_static_queues *sq)
{
	struct amp_static_queue_def queue_def = VIRTIO_MSG_QUEUE_DEF_INIT;
	struct amp_static_queue_head queue_head = {.status = AMP_STATIC_QUEUE_READY};
	uint16_t size;
	int len;

	queue_def.dev_elt_size = cfg->dev_elt_size;
	queue_def.dev_num_elts = cfg->dev_num_elts;
	queue_def.drv_elt_size = cfg->drv_elt_size;
	queue_def.drv_num_elts = cfg->drv_num_elts;

	/* Start address of the device queue is just after the amp_static_queue_def structure */
	queue_def.dev_head = sizeof(queue_def);
	queue_def.dev_data = queue_def.dev_head + sizeof(queue_head);
	if (cfg->drv_queue_off) {
		queue_def.drv_head = cfg->drv_queue_off;
	} else {
		queue_def.drv_head = queue_def.dev_data + cfg->dev_elt_size * cfg->dev_num_elts;
		queue_def.drv_head = ALIGN_64(queue_def.drv_head);
	}

	queue_def.drv_data = queue_def.drv_head + sizeof(queue_head);

	size = queue_def.drv_data + cfg->drv_elt_size * cfg->drv_num_elts;
	size = ALIGN_64(size);

	if (size > io->size)
		return -ENOMEM;

	sq->rx.elt_size = queue_def.drv_num_elts;
	sq->rx.num_elts = queue_def.drv_num_elts;
	sq->rx.head = queue_def.drv_head;
	sq->rx.data = queue_def.drv_data;
	sq->tx.elt_size = queue_def.dev_num_elts;
	sq->tx.num_elts = queue_def.dev_num_elts;
	sq->tx.head = queue_def.dev_head;
	sq->tx.data = queue_def.dev_data;
	sq->io = io;

	/* Initialise the amp_static_queue_def in shared memory */
	len = metal_io_block_write(io, 0, &queue_def, sizeof(queue_def));
	if (len != sizeof(queue_def))
		return -EINVAL;

	/* Initialise the amp_static_queue_def in shared memory */
	len = metal_io_block_write(io, sq->tx.head, &queue_head, sizeof(queue_head));
	if (len != sizeof(queue_head))
		return -EINVAL;

	return 0;
}

int queues_drv_init(struct metal_io_region *io, struct amp_static_queue_def *queue_def,
		    struct amp_static_queues *sq)
{
	struct amp_static_queue_head queue_head = {.status = AMP_STATIC_QUEUE_READY};
	int len;

	/* Initialise the amp_static_queue_def from shared memory */
	len = metal_io_block_read(io, 0, queue_def, sizeof(*queue_def));
	if (len != sizeof(*queue_def))
		return -EINVAL;

	sq->io = io;
	sq->tx.elt_size = queue_def->drv_num_elts;
	sq->tx.num_elts = queue_def->drv_num_elts;
	sq->tx.head = queue_def->drv_head;
	sq->tx.data = queue_def->drv_data;
	sq->rx.elt_size = queue_def->dev_num_elts;
	sq->rx.num_elts = queue_def->dev_num_elts;
	sq->rx.head = queue_def->dev_head;
	sq->rx.data = queue_def->dev_data;

	len = metal_io_block_write(io, sq->tx.head, &queue_head, sizeof(queue_head));
	if (len != sizeof(queue_head))
		return -EINVAL;

	return 0;
}
