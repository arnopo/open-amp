/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2024, Linaro LTD
 * Copyright (c) 2024, STMicroelectronics.
 */

#ifndef _AMP_STATIC_QUEUE_H_
#define _AMP_STATIC_QUEUE_H_

#include <metal/io.h>
#include <stdint.h>
#include <stddef.h>
#include <metal/utilities.h>

#define AMP_STATIC_QUEUE_READY metal_bit(0)

#define MAGIC_Q_DEF 0x1A2B3C4D

#define AMP_STATIC_DRV_ROLE 0
#define AMP_STATIC_DEV_ROLE 0

/* AMP shared memory structures */

/**
 *
 * status indicates when this peer is ready for communication to start.
 * if status does not indicate ready, the other elements should be ignored.
 *
 * head is an element index that points at the next element to be written by
 * this peer in its own data area.  head will always point to a free element.
 *
 * tail is an element index that points at the last element read by this peer
 * from the other peers data area.
 *
 * head and tail wrap-around at element size,
 * each should always be < elt_size
 *
 * my_q->head == other_q->tail means empty
 * my_q->head == WRAP(other_q->tail - 1) means full
 *
 */
struct amp_static_queue_head {
	uint16_t	status;		/* magic & ready indication */
	uint16_t	resv;		/* reserved for now, maybe restart detect */
	uint16_t	w_idx;		/* write index for my queue */
	uint16_t	r_idx;		/* read index for other queue */
};

/**
 * @brief  This structure can be used in the start of shared memory to define the
 * shared memory layout for a queue pair.
 *
 * This structure is static at run time from the device & drv POV
 *
 * Use of this structure is not required if both sides can get this information
 * from other places such as device tree or compile time constants.
 */
struct amp_static_queue_def {
	uint32_t magic;		  /* magic & ready indication
				   *    ready = MAGIC_Q_DEF
				   *    not ready = 0 or MAGIC_Q_DEF_NOT_READY
				   */
	uint32_t version;	  /* fixed to 1 */
	uint32_t driver_peer_ord; /* should be 0 if not used */
	uint32_t dev_peer_ord;	  /* should be 1 if not used */

	/**
	 * element size should be a power of 2,
	 *    some implementations may only support a given size such as 64
	 * number of elements should be > 1
	 */
	uint16_t drv_elt_size;
	uint16_t drv_num_elts;

	/**
	 * size and count of queue elements from device to driver
	 * same constraints as driver to device
	 */
	uint16_t dev_elt_size;
	uint16_t dev_num_elts;

	/**
	 * offsets from the start of the containing memory area
	 * each head is of type amp_queue_head_t
	 * each data is u8 data[num_elts][elt_size]
	 */
	uint64_t drv_head;
	uint64_t drv_data;
	uint64_t dev_head;
	uint64_t dev_data;
};

struct amp_static_queue {
	uint16_t elt_size;
	uint16_t num_elts;
	uint64_t head;
	uint64_t data;
};

struct amp_static_queues {
	struct amp_static_queue	rx;
	struct amp_static_queue	tx;
	struct metal_io_region	*io;
};

struct amp_static_queues_cfg {
	uint16_t	dev_elt_size;
	uint16_t	dev_num_elts;
	uint16_t	drv_elt_size;
	uint16_t	drv_num_elts;
	uint64_t	drv_queue_off; /* specify the drv queue start offset */
};

int amp_static_queue_dev_init(struct metal_io_region *io, const struct amp_static_queues_cfg *cfg,
			      struct amp_static_queues *sq);
int amp_static_queue_drv_init(struct metal_io_region *io, struct amp_static_queue_def *queue_def,
			      struct amp_static_queues *sq);
int amp_static_queue_send(struct amp_static_queues *q, void *msg, size_t msg_len);
int amp_static_queue_receive(struct amp_static_queues *q, void *msg, size_t msg_len);
int amp_static_queue_connect(struct metal_io_region *io, struct amp_static_queues *sq);

#endif /* _AMP_STATIC_QUEUE_H_ */
